#include "context.hpp"
#include "camera.hpp"
#include "model.hpp"
#include "shader.hpp"

#include <GL/glew.h>
#include <imgui.h>
#include <iostream>
#include <stdexcept>
#include <memory>
#include <functional>

int main()
{
    std::shared_ptr<Context> con;
    std::shared_ptr<Camera> cam;
    std::shared_ptr<Shader> render;

    // initialze all
    try
    {
        con = std::make_shared<Context>("Marker");
        cam = std::make_shared<Camera>();
        render = std::make_shared<Shader>();
        render->add("shaders/render.vert.glsl", GL_VERTEX_SHADER);
        render->add("shaders/render.frag.glsl", GL_FRAGMENT_SHADER);
        render->compile();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return -1;
    }

    // create UI function
    auto renderUI = [&]()
    {
        ImGui::SetNextWindowSize({300.0f, 200.0f}, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos({10.0f, 10.0f}, ImGuiCond_FirstUseEver);
        ImGui::Begin("UI");
        if(ImGui::BeginTabBar("Configs"))
        {
            if(ImGui::BeginTabItem("Context"))
            {
                con->UI();
                ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem("Camera"))
            {
                cam->UI();
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
        ImGui::End();
    };

    // main loop
    while(con->loop())
    {
        con->beginFrame();
        cam->update();
        // render camera frome to screen
        glUseProgram(render->program());
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, cam->texture());
        render->uniformInt("image", 0);
        render->uniformFloat("ratio_img", cam->ratio());
        render->uniformFloat("ratio_win", con->ratio());
        cam->draw();
        glUseProgram(0);
        con->endFrame(renderUI);
    }

    return 0;
}