#include "Mobitech.h"
#include "Renderer.h"
#include <time.h>

#ifdef MOBITECH_WIN32
#include <sys/timeb.h>
#include <windows.h>
#endif //MOBITECH_WIN32

ResourceFactory Engine::main_resource_factory;

Engine* Engine::GetInstance()
{
    if(instance == NULL)
        instance = new Engine();        
    return instance;
}

bool Engine::Initialize()
{
    elapsed_time = 0;
    fps = 0;

    if(Window::IsRunning())
        return true;

    Logger::Message("Start application");
    return Renderer::GetInstance()->Initialize();
}

void Engine::Stop()
{
    Window::SetRunning(false);
}

long long Engine::GetTimeMS()
{

#ifdef MOBITECH_WIN32
   SYSTEMTIME time;
   GetSystemTime(&time);
   return time.wMilliseconds;
#endif

#ifdef MOBITECH_ANDROID
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000LL + tv.tv_usec / 1000;
#endif //MOBITECH_WIN32    
}

void Engine::OneFrame()
{
    begin_frame_time = GetTimeMS();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
	Renderer::GetInstance()->draw_calls = 0;

    if(current_scene.get() != NULL)
        current_scene->DrawFrame();

    double currentTick = 0;
#ifdef MOBITECH_WIN32
    SwapBuffers(Window::GetHDC());
#endif //MOBITECH_WIN32

    currentTick = GetTimeMS();
    delta_time += currentTick - begin_frame_time;

    if(current_scene.get() != NULL)
        current_scene->Update((currentTick - begin_frame_time)/1000);

    elapsed_time += (float)(currentTick - begin_frame_time)/1000.0f;
    ++fps;
}

void Engine::Run()
{
    #ifdef MOBITECH_WIN32
    MSG msg;
    
    Window::SetActive(true);
    Window::SetRunning(true); 	

    delta_time      = 0.0;
    fixed_time_step  = 1.0 / 100.0;

    while (Window::IsRunning())
    {
        while (PeekMessage(&msg, Window::GetHWND(), 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
			{
                Window::SetRunning(false);
                break;
            }
            DispatchMessage(&msg);
        }
        
        begin_frame_time = GetTimeMS();

        //if (Input::IsKeyPressed(VK_ESCAPE))
          //  this->Stop();

        if (Window::IsRunning() && Window::IsActive())
        {
            OneFrame();

            if (elapsed_time >= 1.0f)
            {
                char buff[50];
                sprintf_s(buff, "Mobitech FPS: %u, Draw Calls: %u", fps, Renderer::GetInstance()->draw_calls);

                Window::SetWindowTitle(buff);
                fps = 0;
                elapsed_time = 0.0f;
            }
        }
    }

    Window::SetActive(false);
    Window::SetRunning(false);
    #endif //MOBITECH_WIN32

#ifdef MOBITECH_ANDROID
    delta_time = 0.0;
    fixed_time_step = 1.0/ 100.0;    
#endif // MOBITECH_ANDROID
}

void Engine::Release()
{
    main_resource_factory.ReleaseAll();
    Renderer::GetInstance()->Release();
    Renderer::Free();
    Logger::Free();
}