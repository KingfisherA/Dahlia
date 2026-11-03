#include <algorithm>
#include <cstdlib>
#include <stdexcept>

#include <spirv_msl.hpp>

#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>

#include "Dahlia/Platform/Metal/MetalContext.h"
#include "Dahlia/Platform/Metal/MetalShaderModule.h"
#include "Dahlia/Utilities/Assert.h"

namespace DH
{
    MetalShaderModule::MetalShaderModule(ShaderStage stage, std::span<const uint32_t> spirv)
    {
        mReflection = ShaderModule::Reflect(stage, spirv);

        // Assign argument indices in (set, binding) order
        std::vector<ShaderResource> resources = mReflection.resources;
        std::sort(resources.begin(),
                  resources.end(),
                  [](const ShaderResource& a, const ShaderResource& b)
                  { return a.set != b.set ? a.set < b.set : a.binding < b.binding; });

        const spv::ExecutionModel model =
          stage == ShaderStage::Vertex ? spv::ExecutionModelVertex : spv::ExecutionModelFragment;
        spirv_cross::CompilerMSL compiler(spirv.data(), spirv.size());

        uint32_t nextBuffer = PUSH_CONSTANT_BUFFER_INDEX + 1, nextTexture = 0;
        for (const ShaderResource& resource : resources)
        {
            spirv_cross::MSLResourceBinding binding;
            binding.stage = model;
            binding.desc_set = resource.set;
            binding.binding = resource.binding;
            binding.count = 1;

            ArgumentSlot slot{ resource.set, resource.binding, resource.type, 0 };
            if (resource.type == ShaderResourceType::UniformBuffer)
            {
                slot.index = binding.msl_buffer = nextBuffer++;
                if (slot.index >= VERTEX_BUFFER_BASE)
                    throw std::runtime_error("Too many uniform buffers in one Metal shader stage");
            }
            else
            {
                slot.index = binding.msl_texture = binding.msl_sampler = nextTexture++;
            }
            compiler.add_msl_resource_binding(binding);
            mSlots.push_back(slot);
        }

        spirv_cross::MSLResourceBinding pushBinding;
        pushBinding.stage = model;
        pushBinding.desc_set = spirv_cross::kPushConstDescSet;
        pushBinding.binding = spirv_cross::kPushConstBinding;
        pushBinding.count = 1;
        pushBinding.msl_buffer = PUSH_CONSTANT_BUFFER_INDEX;
        compiler.add_msl_resource_binding(pushBinding);

        spirv_cross::CompilerGLSL::Options common;
        common.vertex.fixup_clipspace = true; // OpenGL [-1, 1] depth -> Metal [0, 1]
        common.vertex.flip_vert_y = false;    // Metal NDC +y is already up
        compiler.set_common_options(common);

        spirv_cross::CompilerMSL::Options options;
        options.platform = spirv_cross::CompilerMSL::Options::macOS;
        options.set_msl_version(2, 4);
        compiler.set_msl_options(options);

        mMSLSource = compiler.compile();
        const std::string entryPoint = compiler.get_cleansed_entry_point_name("main", model);

#ifdef CONFIG_DEBUG
        if (const char* dump = std::getenv("DH_DUMP_MSL"); dump != nullptr && dump[0] == '1')
        {
            DH_LOG(Blue, DH::MetalShaderModule, "MSL:\n" << mMSLSource);
        }
#endif

        MTL::Device* device = GPUContext::Get<MetalGPUContext>()->GetMetalDevice();
        NS::Error* error = nullptr;
        mLibrary = device->newLibrary(
          NS::String::string(mMSLSource.c_str(), NS::UTF8StringEncoding), nullptr, &error);
        if (mLibrary == nullptr)
        {
            DH_FATAL(DH::MetalShaderModule,
                     "Generated MSL failed to compile:\n"
                       << (error ? error->localizedDescription()->utf8String() : "unknown error")
                       << "\n--- Generated source ---\n"
                       << mMSLSource);
            throw std::runtime_error("Metal shader compilation failed");
        }

        mFunction =
          mLibrary->newFunction(NS::String::string(entryPoint.c_str(), NS::UTF8StringEncoding));
        if (mFunction == nullptr)
            throw std::runtime_error("Metal shader entry point '" + entryPoint + "' not found");
    }

    MetalShaderModule::~MetalShaderModule()
    {
        if (mFunction != nullptr)
            mFunction->release();
        if (mLibrary != nullptr)
            mLibrary->release();
    }

    const MetalShaderModule::ArgumentSlot* MetalShaderModule::FindSlot(uint32_t set,
                                                                       uint32_t binding) const
    {
        for (const ArgumentSlot& slot : mSlots)
        {
            if (slot.set == set && slot.binding == binding)
                return &slot;
        }
        return nullptr;
    }
}
