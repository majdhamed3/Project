#include "libs/glad_loader_docs.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include "PolygonWithTexture.cpp"
#include "Polygon.cpp"
#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include "stb_image.h"

using namespace glm;

int SCR_WIDTH = 1280;
int SCR_HEIGHT = 720;

Camera camera(vec3(0.0f, 9.0f, 1.2f));
float deltaTime = 0.0f;
float lastFrame = 0.0f;

void processInput(GLFWwindow *window);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);

std::vector<Polygon> coloredQuads;            
std::vector<PolygonWithTexture> texturedQuads;

GLuint loadTexture(const char *path, bool repeat = true)
{
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (data)
    {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture: " << path << std::endl;
    }
    stbi_image_free(data);
    return texture;
}

void addColoredQuad(std::vector<vec3> verts, vec3 color)
{
    coloredQuads.emplace_back(verts, color);
}


void addTexturedQuad(std::vector<vec3> verts, std::vector<vec2> uvs, GLuint texture, vec3 tint = vec3(1.0f))
{
    texturedQuads.emplace_back(verts, uvs, tint);
    texturedQuads.back().setTexture(texture);
}


void addTexturedBox(vec3 center, vec3 size, GLuint texture,
                    vec3 tintTop = vec3(1.0f), vec3 tintSide = vec3(0.75f), vec3 tintBottom = vec3(0.45f))
{
    float x0 = center.x - size.x / 2, x1 = center.x + size.x / 2;
    float y0 = center.y - size.y / 2, y1 = center.y + size.y / 2;
    float z0 = center.z - size.z / 2, z1 = center.z + size.z / 2;

    
    addTexturedQuad({vec3(x0, y1, z0), vec3(x0, y1, z1), vec3(x1, y1, z1), vec3(x1, y1, z0)},
                    {vec2(0, 0), vec2(0, 1), vec2(1, 1), vec2(1, 0)}, texture, tintTop);
    
    addTexturedQuad({vec3(x0, y0, z0), vec3(x1, y0, z0), vec3(x1, y0, z1), vec3(x0, y0, z1)},
                    {vec2(0, 0), vec2(1, 0), vec2(1, 1), vec2(0, 1)}, texture, tintBottom);

    addTexturedQuad({vec3(x0, y0, z1), vec3(x1, y0, z1), vec3(x1, y1, z1), vec3(x0, y1, z1)},
                    {vec2(0, 0), vec2(1, 0), vec2(1, 1), vec2(0, 1)}, texture, tintSide);

    addTexturedQuad({vec3(x0, y0, z0), vec3(x0, y1, z0), vec3(x1, y1, z0), vec3(x1, y0, z0)},
                    {vec2(0, 0), vec2(0, 1), vec2(1, 1), vec2(1, 0)}, texture, tintSide);
    
    addTexturedQuad({vec3(x0, y0, z0), vec3(x0, y0, z1), vec3(x0, y1, z1), vec3(x0, y1, z0)},
                    {vec2(0, 0), vec2(1, 0), vec2(1, 1), vec2(0, 1)}, texture, tintSide);
    
    addTexturedQuad({vec3(x1, y0, z0), vec3(x1, y1, z0), vec3(x1, y1, z1), vec3(x1, y0, z1)},
                    {vec2(0, 0), vec2(0, 1), vec2(1, 1), vec2(1, 0)}, texture, tintSide);
}


void addColoredBox(vec3 center, vec3 size, vec3 colorTop, vec3 colorSide)
{
    float x0 = center.x - size.x / 2, x1 = center.x + size.x / 2;
    float y0 = center.y - size.y / 2, y1 = center.y + size.y / 2;
    float z0 = center.z - size.z / 2, z1 = center.z + size.z / 2;

    addColoredQuad({vec3(x0, y1, z0), vec3(x0, y1, z1), vec3(x1, y1, z1), vec3(x1, y1, z0)}, colorTop);
    addColoredQuad({vec3(x0, y0, z0), vec3(x1, y0, z0), vec3(x1, y0, z1), vec3(x0, y0, z1)}, colorSide * 0.6f);
    addColoredQuad({vec3(x0, y0, z1), vec3(x1, y0, z1), vec3(x1, y1, z1), vec3(x0, y1, z1)}, colorSide);
    addColoredQuad({vec3(x0, y0, z0), vec3(x0, y1, z0), vec3(x1, y1, z0), vec3(x1, y0, z0)}, colorSide);
    addColoredQuad({vec3(x0, y0, z0), vec3(x0, y0, z1), vec3(x0, y1, z1), vec3(x0, y1, z0)}, colorSide);
    addColoredQuad({vec3(x1, y0, z0), vec3(x1, y1, z0), vec3(x1, y1, z1), vec3(x1, y0, z1)}, colorSide);
}


void addColoredCone(vec3 baseCenter, float radius, float height, vec3 color, int segments = 12)
{
    vec3 apex = baseCenter + vec3(0.0f, height, 0.0f);
    for (int i = 0; i < segments; i++)
    {
        float a0 = (float)i / (float)segments * 6.2831853f;
        float a1 = (float)(i + 1) / (float)segments * 6.2831853f;
        vec3 b0 = baseCenter + vec3(cos(a0) * radius, 0.0f, sin(a0) * radius);
        vec3 b1 = baseCenter + vec3(cos(a1) * radius, 0.0f, sin(a1) * radius);
        float c = cos((a0 + a1) * 0.5f - 0.6f);
        float shade = 0.65f + 0.35f * (c > 0.0f ? c : 0.0f);
        addColoredQuad({b0, b1, apex, apex}, color * shade);
    }
}


void addTableFrame(float cx, float cz, float w, float d, float y, float thick, vec3 color)
{
    addColoredBox(vec3(cx, y, cz - d / 2), vec3(w, 0.02f, thick), color, color); 
    addColoredBox(vec3(cx, y, cz + d / 2), vec3(w, 0.02f, thick), color, color); 
    addColoredBox(vec3(cx - w / 2, y, cz), vec3(thick, 0.02f, d), color, color); 
    addColoredBox(vec3(cx + w / 2, y, cz), vec3(thick, 0.02f, d), color, color); 
}


enum CardLocation { IN_DECK, MOVING, PLAYED };

struct Card
{
    PolygonWithTexture quad;
    CardLocation location = IN_DECK;
    vec3 currentPos;
    vec3 startPos;
    vec3 targetPos;
    float moveT = 1.0f; 

    Card(std::vector<vec3> v, std::vector<vec2> t, GLuint texture, vec3 pos)
        : quad(v, t), currentPos(pos)
    {
        quad.setTexture(texture);
    }
};

const float CARD_W = 0.9f;
const float CARD_H = 1.3f;
const float TABLE_TOP_Y = 1.5f;
const float CARD_Y = TABLE_TOP_Y + 0.06f;
const float STAND_ANGLE = 80.0f; 

std::vector<Card> cards;


vec3 camPos[2] = {vec3(0.0f, 3.5f, 0.3f), vec3(0.0f, 3.4f, 5.8f)};
vec3 camFront[2] = {normalize(vec3(0.0f, -1.0f, -0.13f)), normalize(vec3(0.0f, -0.34f, -1.0f))};
int camIndex = 0;
vec3 camPosFrom = camPos[0], camFrontFrom = camFront[0];
float camAnimT = 1.0f; 


float tiltCurrent = 0.0f;
float tiltTarget = 0.0f;

int cardsPlayed = 0;

vec3 playedSlot(int i)
{
    return vec3(-2.4f + 1.2f * i, CARD_Y, 0.0f);
}

vec3 deckSlot(int i)
{
    return vec3(-2.4f + 1.2f * i, CARD_Y, 1.35f);
}

void switchCamera(int idx)
{
    if (idx == camIndex && camAnimT >= 1.0f)
        return;
    camPosFrom = camera.Position;
    camFrontFrom = camera.Front;
    camIndex = idx;
    camAnimT = 0.0f;
    tiltTarget = (idx == 1) ? 1.0f : 0.0f;
}

void playRandomCard()
{
    std::vector<int> remaining;
    for (int i = 0; i < (int)cards.size(); i++)
        if (cards[i].location == IN_DECK)
            remaining.push_back(i);

    if (remaining.empty())
    {
        std::cout << "No cards left in the deck - nothing happens." << std::endl;
        return;
    }

    int pick = remaining[rand() % remaining.size()];
    Card &c = cards[pick];
    c.location = MOVING;
    c.startPos = c.currentPos;
    c.targetPos = playedSlot(cardsPlayed);
    c.moveT = 0.0f;
    cardsPlayed++;
    std::cout << "Card " << pick + 1 << " moves to the played area!" << std::endl;
}


int main()
{
    srand((unsigned int)time(nullptr));

    glfwInit();
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Fantasy Card Game - CG Project", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    glEnable(GL_DEPTH_TEST);

    stbi_set_flip_vertically_on_load(true);

    Shader ourShader("./shaders/vs/shader.vs", "./shaders/fs/shader.fs");

    
    GLuint texWood = loadTexture("./textures/wood_floor.png");
    GLuint texWallpaper = loadTexture("./textures/wallpaper.png");
    GLuint texPoster = loadTexture("./textures/poster.png", false);
    GLuint texCard[5];
    texCard[0] = loadTexture("./textures/card_minato.png", false);
    texCard[1] = loadTexture("./textures/card_naruto.png", false);
    texCard[2] = loadTexture("./textures/card_sasuke.png", false);
    texCard[3] = loadTexture("./textures/card_itachi.png", false);
    texCard[4] = loadTexture("./textures/card_tobirama.png", false);

    
    addTexturedQuad({vec3(-7, 0, -6), vec3(-7, 0, 6), vec3(7, 0, 6), vec3(7, 0, -6)},
                    {vec2(0, 0), vec2(0, 4), vec2(4, 4), vec2(4, 0)}, texWood);
    
    addColoredQuad({vec3(-7, 4, -6), vec3(7, 4, -6), vec3(7, 4, 6), vec3(-7, 4, 6)}, vec3(0.16f, 0.13f, 0.11f));
   
    addTexturedQuad({vec3(-7, 0, -6), vec3(-7, 4, -6), vec3(7, 4, -6), vec3(7, 0, -6)},
                    {vec2(0, 0), vec2(0, 1.4f), vec2(3.5f, 1.4f), vec2(3.5f, 0)}, texWallpaper, vec3(0.85f));
    
    addTexturedQuad({vec3(-7, 0, 6), vec3(7, 0, 6), vec3(7, 4, 6), vec3(-7, 4, 6)},
                    {vec2(0, 0), vec2(3.5f, 0), vec2(3.5f, 1.4f), vec2(0, 1.4f)}, texWallpaper, vec3(0.8f));
   
    addTexturedQuad({vec3(-7, 0, -6), vec3(-7, 0, 6), vec3(-7, 4, 6), vec3(-7, 4, -6)},
                    {vec2(0, 0), vec2(3, 0), vec2(3, 1.4f), vec2(0, 1.4f)}, texWallpaper, vec3(0.75f));
   
    addTexturedQuad({vec3(7, 0, -6), vec3(7, 4, -6), vec3(7, 4, 6), vec3(7, 0, 6)},
                    {vec2(0, 0), vec2(0, 1.4f), vec2(3, 1.4f), vec2(3, 0)}, texWallpaper, vec3(0.75f));

    
    addTexturedBox(vec3(0, TABLE_TOP_Y - 0.075f, 0), vec3(7.0f, 0.15f, 4.0f), texWood,
                   vec3(0.9f, 0.75f, 0.6f), vec3(0.55f, 0.4f, 0.3f));
    
    vec3 legColor(0.25f, 0.16f, 0.1f);
    addColoredBox(vec3(-3.2f, 0.675f, -1.7f), vec3(0.25f, 1.35f, 0.25f), legColor, legColor);
    addColoredBox(vec3(3.2f, 0.675f, -1.7f), vec3(0.25f, 1.35f, 0.25f), legColor, legColor);
    addColoredBox(vec3(-3.2f, 0.675f, 1.7f), vec3(0.25f, 1.35f, 0.25f), legColor, legColor);
    addColoredBox(vec3(3.2f, 0.675f, 1.7f), vec3(0.25f, 1.35f, 0.25f), legColor, legColor);

    
    addTableFrame(0, -1.35f, 6.4f, 1.1f, TABLE_TOP_Y + 0.01f, 0.06f, vec3(0.45f, 0.12f, 0.12f)); // opponent
    addTableFrame(0, 0.0f, 6.4f, 1.1f, TABLE_TOP_Y + 0.01f, 0.06f, vec3(0.15f, 0.35f, 0.15f));   // played
    addTableFrame(0, 1.35f, 6.4f, 1.1f, TABLE_TOP_Y + 0.01f, 0.06f, vec3(0.12f, 0.2f, 0.45f));   // deck

    
    
    std::vector<vec3> cardVerts = {
        vec3(-CARD_W / 2, 0, -CARD_H / 2),
        vec3(CARD_W / 2, 0, -CARD_H / 2),
        vec3(CARD_W / 2, 0, CARD_H / 2),
        vec3(-CARD_W / 2, 0, CARD_H / 2)};
    std::vector<vec2> cardUVs = {vec2(0, 1), vec2(1, 1), vec2(1, 0), vec2(0, 0)};

    for (int i = 0; i < 5; i++)
        cards.emplace_back(cardVerts, cardUVs, texCard[i], deckSlot(i));

    
    {
        float px = -2.6f, py = 2.55f, pz = -5.98f; 
        float pw = 1.6f, ph = 2.2f;                
        std::vector<vec2> outline = {              
            {-pw / 2, -ph / 2}, {pw / 2, -ph / 2},
            {0.80f, 0.50f}, {0.62f, 0.58f}, {0.70f, 0.70f}, {0.52f, 0.76f},
            {0.60f, 0.88f}, {0.42f, 0.95f}, {0.30f, ph / 2},
            {-pw / 2, ph / 2},
            {-pw / 2, -ph / 2}}; 
        std::vector<vec3> pv;
        std::vector<vec2> puv;
        pv.push_back(vec3(px, py, pz)); 
        puv.push_back(vec2(0.5f, 0.5f));
        for (auto &o : outline)
        {
            pv.push_back(vec3(px + o.x, py + o.y, pz));
            puv.push_back(vec2((o.x + pw / 2) / pw, (o.y + ph / 2) / ph));
        }
        addTexturedQuad(pv, puv, texPoster, vec3(0.9f));
    }

    
    vec3 wood(0.4f, 0.27f, 0.15f), woodDark(0.3f, 0.2f, 0.11f);
    addColoredBox(vec3(5.6f, 1.5f, -5.6f), vec3(3.0f, 3.0f, 0.6f), wood, woodDark);          // body
    addColoredBox(vec3(5.6f, 1.6f, -5.28f), vec3(2.7f, 2.6f, 0.06f), woodDark, woodDark);     // back panel
    addColoredBox(vec3(5.6f, 1.0f, -5.35f), vec3(2.7f, 0.08f, 0.45f), wood, wood);            // shelf 1
    addColoredBox(vec3(5.6f, 1.9f, -5.35f), vec3(2.7f, 0.08f, 0.45f), wood, wood);            // shelf 2
    
    vec3 bookCols[6] = {vec3(0.55f, 0.15f, 0.15f), vec3(0.15f, 0.3f, 0.55f), vec3(0.2f, 0.45f, 0.2f),
                        vec3(0.6f, 0.45f, 0.15f), vec3(0.4f, 0.15f, 0.45f), vec3(0.7f, 0.35f, 0.2f)};
    for (int i = 0; i < 6; i++)
    {
        float bx = 4.6f + 0.32f * i;
        float by = (i < 3) ? 1.32f : 2.22f; 
        addColoredBox(vec3(bx, by, -5.35f), vec3(0.22f, 0.55f, 0.32f), bookCols[i], bookCols[i] * 0.8f);
    }

   
    addColoredBox(vec3(-5.6f, 0.06f, -4.6f), vec3(0.6f, 0.12f, 0.6f), vec3(0.2f, 0.18f, 0.16f), vec3(0.18f, 0.16f, 0.14f)); // base
    addColoredBox(vec3(-5.6f, 1.35f, -4.6f), vec3(0.09f, 2.6f, 0.09f), vec3(0.3f, 0.26f, 0.2f), vec3(0.28f, 0.24f, 0.18f)); // pole
    addColoredBox(vec3(-5.6f, 2.85f, -4.6f), vec3(0.85f, 0.6f, 0.85f), vec3(1.0f, 0.85f, 0.55f), vec3(0.95f, 0.78f, 0.45f)); // warm shade

    
    vec3 pine(0.18f, 0.65f, 0.22f);
    addColoredBox(vec3(-4.3f, 0.15f, -4.5f), vec3(0.35f, 0.3f, 0.35f),
                  vec3(0.55f, 0.3f, 0.18f), vec3(0.45f, 0.24f, 0.14f));  
    addColoredBox(vec3(-4.3f, 0.45f, -4.5f), vec3(0.16f, 0.5f, 0.16f),
                  vec3(0.4f, 0.26f, 0.14f), vec3(0.34f, 0.22f, 0.12f));  
    addColoredCone(vec3(-4.3f, 0.55f, -4.5f), 0.75f, 1.1f, pine);        
    addColoredCone(vec3(-4.3f, 1.25f, -4.5f), 0.55f, 0.9f, pine);        
    addColoredCone(vec3(-4.3f, 1.85f, -4.5f), 0.38f, 0.75f, pine);       

    
    vec3 rugMain(0.14f, 0.42f, 0.18f), rugEdge(0.07f, 0.26f, 0.11f), rugGold(0.8f, 0.65f, 0.25f);
    addColoredBox(vec3(0.0f, 0.015f, 0.0f), vec3(8.6f, 0.03f, 5.2f), rugMain, rugMain);   
    addColoredBox(vec3(0.0f, 0.02f, -2.4f), vec3(8.6f, 0.032f, 0.4f), rugEdge, rugEdge);  
    addColoredBox(vec3(0.0f, 0.02f, 2.4f), vec3(8.6f, 0.032f, 0.4f), rugEdge, rugEdge);  
    addColoredBox(vec3(-4.1f, 0.02f, 0.0f), vec3(0.4f, 0.032f, 5.2f), rugEdge, rugEdge);  
    addColoredBox(vec3(4.1f, 0.02f, 0.0f), vec3(0.4f, 0.032f, 5.2f), rugEdge, rugEdge);   
    addColoredBox(vec3(0.0f, 0.022f, -2.12f), vec3(7.8f, 0.034f, 0.07f), rugGold, rugGold); 
    addColoredBox(vec3(0.0f, 0.022f, 2.12f), vec3(7.8f, 0.034f, 0.07f), rugGold, rugGold);  
    addColoredBox(vec3(-3.85f, 0.022f, 0.0f), vec3(0.07f, 0.034f, 4.4f), rugGold, rugGold); 
    addColoredBox(vec3(3.85f, 0.022f, 0.0f), vec3(0.07f, 0.034f, 4.4f), rugGold, rugGold);  
    for (float mx = -3.3f; mx <= 3.31f; mx += 0.82f)   
    {
        addColoredBox(vec3(mx, 0.024f, -2.35f), vec3(0.13f, 0.035f, 0.13f), rugGold, rugGold);
        addColoredBox(vec3(mx, 0.024f, 2.35f), vec3(0.13f, 0.035f, 0.13f), rugGold, rugGold);
    }
    for (float mz = -1.7f; mz <= 1.71f; mz += 0.85f)  
    {
        addColoredBox(vec3(-4.0f, 0.024f, mz), vec3(0.13f, 0.035f, 0.13f), rugGold, rugGold);
        addColoredBox(vec3(4.0f, 0.024f, mz), vec3(0.13f, 0.035f, 0.13f), rugGold, rugGold);
    }

    
    ourShader.use();
    ourShader.setInt("texture_diffuse1", 0);
    mat4 perspProj = perspective(radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    mat4 orthoProj = ortho(-4.3f, 4.3f, -2.42f, 2.42f, 0.1f, 100.0f);
    ourShader.setMat4("projection", orthoProj);

    camera.Position = camPos[0];
    camera.Front = camFront[0];

    
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

       
        if (camAnimT < 1.0f)
        {
            camAnimT = std::min(1.0f, camAnimT + deltaTime / 0.8f);
            float s = camAnimT * camAnimT * (3.0f - 2.0f * camAnimT); // smoothstep
            camera.Position = mix(camPosFrom, camPos[camIndex], s);
            camera.Front = normalize(mix(camFrontFrom, camFront[camIndex], s));
        }

       
        float tiltDir = (tiltTarget > tiltCurrent) ? 1.0f : -1.0f;
        if (tiltCurrent != tiltTarget)
        {
            tiltCurrent += tiltDir * deltaTime / 0.9f;
            tiltCurrent = glm::clamp(tiltCurrent, 0.0f, 1.0f);
        }

       
        for (auto &c : cards)
        {
            if (c.location == MOVING)
            {
                c.moveT = std::min(1.0f, c.moveT + deltaTime / 1.1f);
                float s = c.moveT * c.moveT * (3.0f - 2.0f * c.moveT);
                c.currentPos = mix(c.startPos, c.targetPos, s);
                if (c.moveT >= 1.0f)
                    c.location = PLAYED;
            }
        }

        glClearColor(0.05f, 0.04f, 0.04f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ourShader.use();
        ourShader.setMat4("projection", camIndex == 0 ? orthoProj : perspProj);
        ourShader.setMat4("view", camera.GetViewMatrix());

        mat4 identity = mat4(1.0f);

    
        for (auto &q : texturedQuads)
        {
            q.transformation(identity);
            q.draw(ourShader);
        }
        for (auto &q : coloredQuads)
        {
            q.transformation(identity);
            q.draw(ourShader);
        }

    
        for (auto &c : cards)
        {
            float arc = (c.location == MOVING) ? sin(3.14159f * c.moveT) * 1.1f : 0.0f;
            mat4 m = mat4(1.0f);
            m = translate(m, c.currentPos + vec3(0.0f, arc, 0.0f));
            m = translate(m, vec3(0.0f, 0.0f, CARD_H / 2));
            m = rotate(m, radians(STAND_ANGLE * tiltCurrent), vec3(1.0f, 0.0f, 0.0f));
            m = translate(m, vec3(0.0f, 0.0f, -CARD_H / 2));
            c.quad.transformation(m);
            c.quad.draw(ourShader);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow *window)
{
    static bool gPressed = false, onePressed = false, twoPressed = false;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    
    bool one = glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS;
    if (one && !onePressed)
        switchCamera(0);
    onePressed = one;

    bool two = glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS;
    if (two && !twoPressed)
        switchCamera(1);
    twoPressed = two;


    bool g = glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS;
    if (g && !gPressed)
        playRandomCard();
    gPressed = g;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}
