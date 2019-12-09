#include "gameplay.h"

#include "input/keyboard.h"
#include <SFML/Window/Mouse.hpp>
#include <common/debug.h>
#include <common/network/packet.h>

namespace {
gl::VertexArray createCube()
{
    // Create a cube for opengl testing
    std::vector<GLfloat> vertices = {// Front
                                     1, 2, 1, 
                                     0, 2, 1,
                                     0, 0, 1, 
                                     1, 0, 1,

                                     //Left
                                     0, 2, 1,
                                     0, 2, 0,
                                     0, 0, 0,
                                     0, 0, 1,


                                     //Back
                                     0, 2, 0,
                                     1, 2, 0,
                                     1, 0, 0,
                                     0, 0, 0,


                                     //Right
                                     1, 2, 0,
                                     1, 2, 1,
                                     1, 0, 1,
                                     1, 0, 0,

                                     //Top
                                     1, 2, 0,
                                     0, 2, 0,
                                     0, 2, 1,
                                     1, 2, 1,

                                     //Bottom
                                     
                                     0, 0, 0, 
                                     1, 0, 0, 
                                     1, 0, 1,
                                     0, 0, 1, 



                                     /*
                                     // right
                                     2, 0, 0, 2, 2, 0, 2, 2, 2, 2, 0, 2,
                                     // back
                                     0, 0, 0, 0, 2, 0, 2, 2, 0, 2, 0, 0,
                                     // left
                                     0, 0, 0, 0, 0, 2, 0, 2, 2, 0, 2, 0,
                                     // bottom
                                     0, 0, 2, 0, 0, 0, 2, 0, 0, 2, 0, 2,
                                     // top
                                     0, 2, 2, 2, 2, 2, 2, 2, 0, 0, 2, 0};
                                     */};
    std::vector<GLuint> indices;
    for (int itr = 0, i = 0; itr < 6; itr++) {
        indices.push_back(i);
        indices.push_back(i + 1);
        indices.push_back(i + 2);
        indices.push_back(i + 2);
        indices.push_back(i + 3);
        indices.push_back(i);
        i += 4;
    }
    std::vector<GLfloat> textureCoords;

    for (int i = 0; i < 6; i++) {
        textureCoords.insert(textureCoords.end(),
                             {0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f});
    }

    gl::VertexArray vao;
    vao.create();
    vao.bind();
    vao.addVertexBuffer(3, vertices);
    vao.addVertexBuffer(2, textureCoords);
    vao.addIndexBuffer(indices);

    return vao;
}
} // namespace

void Gameplay::init(float aspect)
{
    m_cube = createCube();

    m_shader.create("static", "static");
    m_shader.bind();
    m_modelLocation = m_shader.getUniformLocation("modelMatrix");
    m_projectionViewLocation =
        m_shader.getUniformLocation("projectionViewMatrix");

    m_texture.create("player");
    m_texture.bind();

    m_client.connectTo(sf::IpAddress::LocalHost);
    m_player = &m_entities[m_client.getClientId()];

    m_projectionMatrix = glm::perspective(3.14f / 2.0f, aspect, 0.01f, 100.0f);
}

void Gameplay::handleInput(const sf::Window &window, const Keyboard &keyboard)
{
    static auto lastMousepositionition = sf::Mouse::getPosition(window);

    if (!m_isMouseLocked && window.hasFocus()) {
        auto change = sf::Mouse::getPosition(window) - lastMousepositionition;
        m_player->rotation.x += static_cast<float>(change.y / 8.0f);
        m_player->rotation.y += static_cast<float>(change.x / 8.0f);
        sf::Mouse::setPosition(
            {(int)window.getSize().x / 2, (int)window.getSize().y / 2}, window);
        lastMousepositionition = sf::Mouse::getPosition(window);
    }

    const float PLAYER_SPEED = 0.05f;
    float rads = (glm::radians(m_player->rotation.y));
    float rads90 = (glm::radians(m_player->rotation.y + 90));
    if (keyboard.isKeyDown(sf::Keyboard::W)) {
        m_player->position.x -= glm::cos(rads90) * PLAYER_SPEED;
        m_player->position.z -= glm::sin(rads90) * PLAYER_SPEED;
    }
    else if (keyboard.isKeyDown(sf::Keyboard::S)) {
        m_player->position.x += glm::cos(rads90) * PLAYER_SPEED;
        m_player->position.z += glm::sin(rads90) * PLAYER_SPEED;
    }
    if (keyboard.isKeyDown(sf::Keyboard::A)) {
        m_player->position.x -= glm::cos(rads) * PLAYER_SPEED;
        m_player->position.z -= glm::sin(rads) * PLAYER_SPEED;
    }
    else if (keyboard.isKeyDown(sf::Keyboard::D)) {
        m_player->position.x += glm::cos(rads) * PLAYER_SPEED;
        m_player->position.z += glm::sin(rads) * PLAYER_SPEED;
    }

    if (keyboard.isKeyDown(sf::Keyboard::Q)) {
        m_player->position.y += 0.1;
    }
    else if (keyboard.isKeyDown(sf::Keyboard::E)) {
        m_player->position.y -= 0.1;
    }
}

void Gameplay::onKeyRelease(sf::Keyboard::Key key)
{
    if (key == sf::Keyboard::L) {
        m_isMouseLocked = !m_isMouseLocked;
    }
}

void Gameplay::update()
{
    // Send position of this player to the server
    auto packet = makePacket(ServerCommand::PlayerPosition);
    auto id = m_client.getClientId();
    auto &entity = m_entities[id];
    packet.data << id << m_entities[id].position.x << m_entities[id].position.y
                << entity.position.z;
    m_client.sendPacketToServer(packet);
    handlePackets();
}

void Gameplay::render()
{
    glm::mat4 viewMatrix{1.0f};
    rotateMatrix(&viewMatrix, m_player->rotation);
    translateMatrix(&viewMatrix, -m_player->position);

    glm::mat4 projectionViewMatrix{1.0f};
    projectionViewMatrix = m_projectionMatrix * viewMatrix;
    gl::loadUniform(m_projectionViewLocation, projectionViewMatrix);

    auto drawable = m_cube.getDrawable();
    drawable.bind();
    for (auto &p : m_entities) {
        if (p.active) {

            glm::mat4 modelMatrix{1.0f};
            translateMatrix(&modelMatrix,
                            {p.position.x, p.position.y, p.position.z});
            gl::loadUniform(m_modelLocation, modelMatrix);
            drawable.draw();
        }
    }

    glm::mat4 modelMatrix{1.0f};
    gl::loadUniform(m_modelLocation, modelMatrix);
    drawable.draw();
}

void Gameplay::endGame()
{
    m_cube.destroy();
    m_texture.destroy();
    m_shader.destroy();

    m_client.disconnect();
}
