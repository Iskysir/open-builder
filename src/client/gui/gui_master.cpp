#include "gui_master.h"

#include "../gl/primitive.h"
#include <glm/gtc/matrix_transform.hpp>

GuiMaster::GuiMaster(float viewportWidth, float viewportHeight)
    : m_quadVao(makeQuadVertexArray(1.f, 1.f))
    , m_viewport(viewportWidth, viewportHeight)
{ // GUI Shader
    m_guiShader.program.create("gui", "gui");
    m_guiShader.program.bind();
    m_guiShader.modelLocation = m_guiShader.program.getUniformLocation("modelMatrix");
    m_guiShader.colorLocation = m_guiShader.program.getUniformLocation("colour");

    m_projection = glm::ortho(0.0f, viewportWidth, 0.0f, viewportHeight, -1.0f, 1.0f);

    m_guiShader.projectionLocation =
        m_guiShader.program.getUniformLocation("projectionMatrix");

    gl::loadUniform(m_guiShader.projectionLocation, m_projection);
}

void GuiMaster::addGui(GuiContainer& container)
{
    m_containers.push_back(&container);
}

void GuiMaster::render()
{
    m_quadVao.bind();
    for (auto container : m_containers) {
        container->render();
    }
}

int GuiMaster::getTexture(const std::string& textureName)
{
    auto itr = m_textureIds.find(textureName);
    if (itr == m_textureIds.end()) {
        return itr->second;
    }
    else {
        int index = m_textures.size();
        gl::Texture2d& texture = m_textures.emplace_back();
        texture.create(textureName);
        m_textureIds.emplace(textureName, index);
        return index;
    }
}