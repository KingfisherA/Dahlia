#pragma once

namespace DH
{
    class GPUContext;
    class MetalDisplay;
}

namespace DH::InfoApplication
{
    class MetalApp
    {
      public:
        MetalApp() {}

        bool Init();
        void Run();
        void Destroy();

      private:
        MetalDisplay* mDisplay = nullptr;
    };
}