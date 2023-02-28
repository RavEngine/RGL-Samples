#pragma once
#include "App.hpp"
#include <RGL/Types.hpp>
#include <RGL/RGL.hpp>
#include <RGL/CommandQueue.hpp>
#include <RGL/Surface.hpp>
#include <RGL/Device.hpp>
#include <RGL/Swapchain.hpp>
#include <RGL/Synchronization.hpp>
#include <RGL/ShaderLibrary.hpp>
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <RGL/Span.hpp>
#include <cstdlib>
#include <chrono>

/**
 @param val an angle in degrees
 @return the angle in radians
 */
inline float deg_to_rad(float val){
    return glm::radians(val);
}

struct Camera{
    glm::vec3 position{0};
    
    float yaw = 0, pitch = 0;
    
    float FOV = 60;
    float nearClip = 0.1;
    float farClip = 100;
    
    static constexpr glm::vec3 vector3_right = glm::vec3(1, 0, 0);
    static constexpr glm::vec3 vector3_up = glm::vec3(0, 1, 0);
    static constexpr glm::vec3 vector3_forward = glm::vec3(0, 0, -1);
    
    glm::mat4 ModelMatrix() const{
        return glm::translate(glm::mat4(1), position) * (glm::toMat4(glm::quat(glm::vec3(0,yaw,0))) * glm::toMat4(glm::quat(glm::vec3(pitch,0,0))));
    }
    
    glm::mat4 GenerateViewMatrix() const{
        return glm::inverse(ModelMatrix());
    }
    
    glm::mat4 GenerateProjectionMatrix(uint32_t width, uint32_t height) const{
        return glm::mat4(glm::perspective(deg_to_rad(FOV), (float)width / height, nearClip, farClip));
    }
    
    glm::mat4 GenerateViewProjMatrix(uint32_t width, uint32_t height){
        return GenerateProjectionMatrix(width, height) * GenerateViewMatrix();
    }
    
    glm::quat getRotation() const{
        return glm::quat(glm::vec3(0,yaw,0)) * glm::quat(glm::vec3(pitch,0,0));
    }
    
    inline glm::vec3 Forward() const{
        return getRotation() * vector3_forward;
    }

    /**
    @return the vector pointing in the up direction of this transform
    */
    inline glm::vec3 Up() const{
        return getRotation() * vector3_up;
    }

    /**
    @return the vector pointing in the right direction of this transform
    */
    inline glm::vec3 Right() const{
        return getRotation() * vector3_right;
    }
};


// all examples aside from HelloTriangle use this for convenience
struct ExampleFramework : public AppBase{
    void init(int argc, char** argv) final;
    void sizechanged(int, int) final;
    void shutdown() final;
    
    virtual void sampleinit(int argc, char** argv) = 0;
    virtual void sampleshutdown() = 0;
    virtual void onresize(int, int) {};
    void onevent(union SDL_Event&) final;
    virtual void sampleevent(union SDL_Event&){};
    void internaltick() final;
    
    constexpr static float camSpeed = 0.1;
    constexpr static float camDecel = 0.9;
    constexpr static float camMaxSpeed = 1;
    constexpr static float mouseSensitivity = 0.005;
    
    struct CameraKeyStates{
        bool forward: 1 = false;
        bool back: 1 = false;
        bool left: 1 = false;
        bool right: 1 = false;
        bool up: 1 = false;
        bool down: 1 = false;
    } cameraKeyStates;

    using clock_t = std::chrono::system_clock;

    std::chrono::time_point<clock_t> applicationLaunchTime = clock_t::now();

    auto getTime() {
        return clock_t::now() - applicationLaunchTime;
    }

    float getTimeSeconds() {
        auto time = getTime();
        return std::chrono::duration_cast<std::chrono::duration<float,std::ratio<1,1>>>(time).count();
    }

    
    // things required by all RGL samples
    RGLDevicePtr device;
    RGLCommandQueuePtr commandQueue;
    RGLSurfacePtr surface;
    RGLSwapchainPtr swapchain;
    RGLFencePtr swapchainFence;

    RGLShaderLibraryPtr GetShader(const std::string& name);
    
    struct stbi_freer{
        void operator()(void* ptr) const{
            free(ptr);
        }
    };
    
    struct TextureData{
        RGL::untyped_owning_span<stbi_freer> bytes;
        uint32_t width, height;
        uint32_t nchannels;
    };
    
    TextureData LoadImage(const std::filesystem::path& path);
    
    Camera camera;
    glm::vec3 camVelocity{0};
    
    int width = 0, height = 0;
};

namespace BasicObjects{
    namespace Cube{
        struct Vertex {
            glm::vec3 pos;
            glm::vec3 normal;
            glm::vec2 uv;
        };
        constexpr static Vertex vertices[] = {
            {{ -1.0,  1.0,  1.0}, { 0.0,  0.0,  1.0}, {0, 0 }},
            {{ 1.0,  1.0,  1.0}, { 0.0,  0.0,  1.0}, {1, 0 }},
            {{ -1.0, -1.0,  1.0}, { 0.0,  0.0,  1.0}, {0, 1 }},
            {{ 1.0, -1.0,  1.0}, { 0.0,  0.0,  1.0}, {1, 1 }},
            {{ -1.0, 1.0, -1.0}, { 0.0,  0.0, -1.0}, {0, 0 }},
            {{ 1.0,  1.0, -1.0}, { 0.0,  0.0, -1.0}, {1, 0 }},
            {{ -1.0, -1.0, -1.0}, { 0.0,  0.0, -1.0}, {0, 1 }},
            {{ 1.0, -1.0, -1.0}, { 0.0,  0.0, -1.0}, {1, 1 }},
            {{ -1.0, 1.0,  1.0}, { 0.0,  1.0,  0.0}, {0, 0 }},
            {{ 1.0,  1.0,  1.0}, { 0.0,  1.0,  0.0}, {1, 0 }},
            {{ -1.0, 1.0, -1.0}, { 0.0,  1.0,  0.0}, {0, 1 }},
            {{ 1.0,  1.0, -1.0}, { 0.0,  1.0,  0.0}, {1, 1 }},
            {{ -1.0, -1.0,  1.0}, { 0.0, -1.0,  0.0}, {0, 0 }},
            {{ 1.0, -1.0,  1.0}, { 0.0, -1.0,  0.0}, {1, 0 }},
            {{ -1.0, -1.0, -1.0}, { 0.0, -1.0,  0.0}, {0, 1 }},
            {{ 1.0, -1.0, -1.0}, { 0.0, -1.0,  0.0}, {1, 1 }},
            {{ 1.0, -1.0,  1.0}, { 1.0,  0.0,  0.0}, {0, 0 }},
            {{ 1.0,  1.0,  1.0}, { 1.0,  0.0,  0.0}, {1, 0 }},
            {{ 1.0, -1.0, -1.0}, { 1.0,  0.0,  0.0}, {0, 1 }},
            {{ 1.0,  1.0, -1.0}, { 1.0,  0.0,  0.0}, {1, 1 }},
            {{ -1.0, -1.0,  1.0}, { -1.0,  0.0,  0.0}, {0, 0 }},
            {{ -1.0, 1.0,  1.0}, { -1.0,  0.0,  0.0}, {1, 0 }},
            {{ -1.0, -1.0, -1.0}, { -1.0,  0.0,  0.0}, {0, 1 }},
            {{ -1.0, 1.0, -1.0}, { -1.0,  0.0,  0.0}, {1, 1 }},
        };
        constexpr static uint32_t indices[] = {
            0,  2,  1,
            1,  2,  3,
            4,  5,  6,
            5,  7,  6,

            8, 9,  10,
            9, 11, 10,
           12, 14, 13,
           13, 14, 15,

           16, 18, 17,
           17, 18, 19,
           20, 21, 22,
           21, 23, 22,
        };
    };
    namespace ScreenTriangle{
        struct Vertex {
            glm::vec2 pos;
        };
        constexpr static Vertex vertices[] = {
            {{0, -10}},
            {{10, 10}},
            {{-10, 10}}
        };
    }
};
