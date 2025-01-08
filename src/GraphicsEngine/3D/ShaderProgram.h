#pragma once

#include <string_view>
#include <string>
#include <unordered_map>
#include <vector>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "GraphicsEngine/GEng_API.h"
#include "GraphicsEngine/Utilities/Memory.h"

namespace GEng {
    GENG_API class ShaderProgram {
    public:
        using ShaderKeyMap = std::unordered_map<std::string, std::string>;

        virtual ~ShaderProgram() = default;

        virtual void Bind() = 0;
        virtual void Unbind() = 0;

        virtual unsigned int GetRendererID() const =0;

        virtual void UploadF(std::string_view name, float val) = 0;
        virtual void UploadVecF(std::string_view name, const glm::vec2 &val) = 0;
        virtual void UploadVecF(std::string_view name, const glm::vec3 &val) = 0;
        virtual void UploadVecF(std::string_view name, const glm::vec4 &val) = 0;
        virtual void UploadMatF(std::string_view name, const glm::mat4 &val) = 0;
        virtual void UploadMatF(std::string_view name, const glm::mat3 &val) = 0;

        virtual void UploadI(std::string_view name, int val) = 0;
        virtual void UploadVecI(std::string_view name, const glm::ivec2 &val) = 0;
        virtual void UploadVecI(std::string_view name, const glm::ivec3 &val) = 0;
        virtual void UploadVecI(std::string_view name, const glm::ivec4 &val) = 0;

        static sRef<ShaderProgram> Create(std::string_view combinedSourceCode);
        static sRef<ShaderProgram> Create(std::string_view vertexSource, std::string_view fragmentSource);
        static sRef<ShaderProgram> CreateFromFile(std::string_view combinedShaderFile);
        static sRef<ShaderProgram> CreateFromFile(std::string_view vertexFile, std::string_view fragmentFile);

        GENG_API struct Preprocessor {
        public:
            static void PreprocessShaderType(const std::string& combinedSourceCode, std::string *vertexOutput,
                                             std::string *fragmentOutput, std::string *geometryOutput);
            static void Preprocess(std::string_view shaderFilePath, std::string &sourceCode,
                                   ShaderKeyMap *insertionKeys);

        private:
            static void EvaluatePreprocessorDirectives(std::string_view shaderFilePath,
                                                       std::vector<std::string> &onceIncludeHeaders,
                                                       std::string &currentLine, std::string &sourceCode,
                                                       std::string_view lastInclude,
                                                       ShaderKeyMap *insertionKeys);
        };
    };
}
