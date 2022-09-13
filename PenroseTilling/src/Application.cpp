#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw_gl3.h"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

#include "Renderer.h"

#include "VertexBufferLayout.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexArray.h"
#include "Shader.h"
#include "Texture.h"
#include "Camera.h"

#include "Penrose.h"


void framebuffer_size_callback( GLFWwindow *window, int width, int height );
void mouse_callback( GLFWwindow *window, double xpos, double ypos );
void scroll_callback( GLFWwindow *window, double xoffset, double yoffset );
void processInput( GLFWwindow *window );

// settings
unsigned int SCR_WIDTH = 1080;
unsigned int SCR_HEIGHT = 810;

// Tilling Settings
const float TILLING_DIAMETER = 1.0f;
const float PARTITIONS = 3;

// camera
Camera camera( glm::vec3( 0.0f, 0.0f, 3.0f ) );
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

int main( void ){
    GLFWwindow *window;

    /* Initialize the library */
    if( !glfwInit() )
        return -1;

    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 4 );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );


    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow( SCR_WIDTH, SCR_HEIGHT, "Proyecto_2", NULL, NULL );
    if( !window ){
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent( window );
    glfwSetFramebufferSizeCallback( window, framebuffer_size_callback );
    glfwSetCursorPosCallback( window, mouse_callback );
    glfwSetScrollCallback( window, scroll_callback );

    // tell GLFW to capture our mouse
    glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_DISABLED );


    glfwSwapInterval( 1 );

    // Se debe poner después del make context eso marca la documentación
    if( glewInit() != GLEW_OK )
        std::cout << "Error!" << std::endl;

    std::cout << glGetString( GL_VERSION ) << std::endl;

    {

        Penrose p( PARTITIONS, Coordinate( 0.0, 0.0 ), 9, TILLING_DIAMETER );

        p.execute();
        p.DoIt3D();

        float *vertices = p.GetVerticesWithColorsTexCoordsAndNormalLight();
        int numVertices = p.GetNumTriangles() * 36;

        int numIndices = p.GetNumTriangles() * 3;

        unsigned int *indices = new unsigned int[ numIndices ];
        for( int i = 0; i < numIndices; i++ ){
            indices[ i ] = i;
        }

        IndexBuffer ib( indices, numIndices );

        std::cout << "tringulos: " << p.GetNumTriangles() << std::endl;
        GLCall( glEnable( GL_BLEND ) );
        GLCall( glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA ) );

        VertexArray va;

        VertexBuffer vb( vertices, numVertices * sizeof( float ) );

        VertexBufferLayout layout;
        layout.Push<float>( 3 );
        layout.Push<float>( 3 );
        layout.Push<float>( 2 );
        layout.Push<float>( 1 );
        layout.Push<float>( 3 );
        va.AddBuffer( vb, layout );
        va.Bind();

        Shader shader( "res/shaders/project.shader" );
        shader.Bind();

        Texture texture_1( "res/textures/nether_brick.png" );
        Texture texture_2( "res/textures/amatista_block.png" );

        GLuint m_texture_1 = texture_1.GetM_RendererID();
        GLuint m_texture_2 = texture_2.GetM_RendererID();
        GLCall( glBindTextureUnit( 2, m_texture_1 ) );
        GLCall( glBindTextureUnit( 1, m_texture_2 ) );
        int samplers[ 3 ] = { 0, 1, 2 };
        shader.Setuniforms1iv( "material.diffuse", 3, samplers );

        va.UnBind();
        shader.UnBind();
        vb.UnBind();

        Renderer renderer;

        glPolygonMode( GL_FRONT, GL_FILL );

        // Setup ImGui binding
        ImGui::CreateContext();
        ImGui_ImplGlfwGL3_Init( window, true );

        // Setup style
        ImGui::StyleColorsDark();

        // Variables
        // for projection
        float near = 3.0;
        float far = -100;

        //for Model
        glm::vec3 translate_Vector( 0.0f, 0.0f, 0.0f );
        float rotate_angle = 0.0f;
        glm::vec3 rotate_Vector( 1.0f, 0.0f, 0.0f );
        glm::vec3 scale_Vector( 1.0, 1.0, 0.5 );

        // For light
        glm::vec3 lightPosition( 0.0, 0.0, 5.0 );
        // positions of the point lights
        glm::vec3 pointLightPositions[] = {
            glm::vec3( 2.7f,  2.2f,  2.0f ),
            glm::vec3( 2.3f, -3.3f, -4.0f ),
            glm::vec3( -4.0f,  2.0f, -10.0f ),
            glm::vec3( 3.0f,  3.0f, -3.0f )
        };

        // For animation
        float explotion_scale = 1.0;
        float magnitude = 6.0;
        float time;
        bool stop_animation = false;
        /* Loop until the user closes the window */
        while( !glfwWindowShouldClose( window ) ){
            // per - frame time logic
            // --------------------
            float currentFrame = glfwGetTime();
            deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;

            /* Render here */
            renderer.Clear();

            ImGui_ImplGlfwGL3_NewFrame();

            processInput( window );

            shader.Bind();

            vb.Bind();
            va.Bind();

            {

                // Projection
                glm::mat4 projection = glm::perspective( glm::radians( camera.Zoom ), ( float ) SCR_WIDTH / ( float ) SCR_HEIGHT, near, far );


                // View
                glm::mat4 view = camera.GetViewMatrix();

                // Model
                glm::mat4 model = glm::mat4( 1.0f );
                model = glm::translate( model, translate_Vector );
                model = glm::rotate( model, rotate_angle, rotate_Vector );
                model = glm::scale( model, scale_Vector );

                // Set uniforms
                shader.SetuniformsMat4f( "projection", projection );
                shader.SetuniformsMat4f( "view", view );
                shader.SetuniformsMat4f( "model", model );
                shader.SetuniformsVec3( "viewPos", camera.Position );

                if( stop_animation ){
                    time = 0;
                } else{

                    time = glfwGetTime();

                }
                if( magnitude > 0 ){

                    shader.SetUniformFloat( "time", 3.14159265359 );
                    shader.SetUniformFloat( "magnitude", magnitude );

                    magnitude -= 0.015;

                } else{

                    shader.SetUniformFloat( "magnitude", 1 * explotion_scale );
                    shader.SetUniformFloat( "time", time );
                }
                /*
                 * Here we set all the uniforms for the 5/6 types of lights we have. We have to set them manually and index
                 * the proper PointLight struct in the array to set each uniform variable. This can be done more code-friendly
                 * by defining light types as classes and set their values in there, or by using a more efficient uniform approach
                 * by using 'Uniform buffer objects', but that is something we'll discuss in the 'Advanced GLSL' tutorial.
                 */
                 // directional light
                shader.SetuniformsVec3( "dirLight.direction", lightPosition );
                shader.SetuniformsVec3( "dirLight.ambient", glm::vec3( 0.05f, 0.05f, 0.05f ) );
                shader.SetuniformsVec3( "dirLight.diffuse", glm::vec3( 0.9f, 0.9f, 0.9f ) );
                shader.SetuniformsVec3( "dirLight.specular", glm::vec3( 0.5f, 0.5f, 0.5f ) );
                // point light 1
                shader.SetuniformsVec3( "pointLights[0].position", pointLightPositions[ 0 ] );
                shader.SetuniformsVec3( "pointLights[0].ambient", glm::vec3( 0.05f, 0.05f, 0.05f ) );
                shader.SetuniformsVec3( "pointLights[0].diffuse", glm::vec3( 0.8f, 0.8f, 0.8f ) );
                shader.SetuniformsVec3( "pointLights[0].specular", glm::vec3( 1.0f, 1.0f, 1.0f ) );
                shader.SetUniformFloat( "pointLights[0].constant", 1.0f );
                shader.SetUniformFloat( "pointLights[0].linear", 0.09 );
                shader.SetUniformFloat( "pointLights[0].quadratic", 0.032 );
                // point light 2
                shader.SetuniformsVec3( "pointLights[1].position", pointLightPositions[ 1 ] );
                shader.SetuniformsVec3( "pointLights[1].ambient", glm::vec3( 0.05f, 0.05f, 0.05f ) );
                shader.SetuniformsVec3( "pointLights[1].diffuse", glm::vec3( 0.8f, 0.8f, 0.8f ) );
                shader.SetuniformsVec3( "pointLights[1].specular", glm::vec3( 1.0f, 1.0f, 1.0f ) );
                shader.SetUniformFloat( "pointLights[1].constant", 1.0f );
                shader.SetUniformFloat( "pointLights[1].linear", 0.09 );
                shader.SetUniformFloat( "pointLights[1].quadratic", 0.032 );
                // point light 3
                shader.SetuniformsVec3( "pointLights[2].position", pointLightPositions[ 2 ] );
                shader.SetuniformsVec3( "pointLights[2].ambient", glm::vec3( 0.05f, 0.05f, 0.05f ) );
                shader.SetuniformsVec3( "pointLights[2].diffuse", glm::vec3( 0.8f, 0.8f, 0.8f ) );
                shader.SetuniformsVec3( "pointLights[2].specular", glm::vec3( 1.0f, 1.0f, 1.0f ) );
                shader.SetUniformFloat( "pointLights[2].constant", 1.0f );
                shader.SetUniformFloat( "pointLights[2].linear", 0.09 );
                shader.SetUniformFloat( "pointLights[2].quadratic", 0.032 );
                // point light 4
                shader.SetuniformsVec3( "pointLights[3].position", pointLightPositions[ 3 ] );
                shader.SetuniformsVec3( "pointLights[3].ambient", glm::vec3( 0.05f, 0.05f, 0.05f ) );
                shader.SetuniformsVec3( "pointLights[3].diffuse", glm::vec3( 0.8f, 0.8f, 0.8f ) );
                shader.SetuniformsVec3( "pointLights[3].specular", glm::vec3( 1.0f, 1.0f, 1.0f ) );
                shader.SetUniformFloat( "pointLights[3].constant", 1.0f );
                shader.SetUniformFloat( "pointLights[3].linear", 0.09 );
                shader.SetUniformFloat( "pointLights[3].quadratic", 0.032 );
                // spotLight
                shader.SetuniformsVec3( "spotLight.position", camera.Position );
                shader.SetuniformsVec3( "spotLight.direction", camera.Front );
                shader.SetuniformsVec3( "spotLight.ambient", glm::vec3( 0.0f, 0.0f, 0.0f ) );
                shader.SetuniformsVec3( "spotLight.diffuse", glm::vec3( 1.0f, 1.0f, 1.0f ) );
                shader.SetuniformsVec3( "spotLight.specular", glm::vec3( 1.0f, 1.0f, 1.0f ) );
                shader.SetUniformFloat( "spotLight.constant", 1.0f );
                shader.SetUniformFloat( "spotLight.linear", 0.09 );
                shader.SetUniformFloat( "spotLight.quadratic", 0.032 );
                shader.SetUniformFloat( "spotLight.cutOff", glm::cos( glm::radians( 12.5f ) ) );
                shader.SetUniformFloat( "spotLight.outerCutOff", glm::cos( glm::radians( 15.0f ) ) );

                // material properties
                shader.SetuniformsVec3( "material.specular", glm::vec3( 0.2f, 0.2f, 0.2f ) );
                shader.SetUniformFloat( "material.shininess", 45.0f );
                // Renderer
                renderer.Draw( va, ib, shader );

            }

            // 1. Show a simple window.
            // Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets automatically appears in a window called "Debug".
            ImGui::Begin( "Controls" );
            {

                ImGui::Text( "Project 2" );
                ImGui::TextWrapped( "\n Camera can moves with AWSD keys. \n" );
                ImGui::TextWrapped( "\n Change camera tu cursor with Q key. \n" );
                ImGui::TextWrapped( "\n Presione E para volver a mover la camara \n" );
                ImGui::TextWrapped( "\nPresiona I para la visualizacon de wireframe\n" );
                ImGui::TextWrapped( "\nPresiona O para la visualizacion con relleno\n" );
                ImGui::TextWrapped( "\nPresiona P para la visualizacion de los puntos que conforman\n" );
                ImGui::TextWrapped( "\nSi se desplaza a la parte de atras de la teslacion prodra ver como se ve el efecto de la luz atraves de las texturas ya que estas son png con un poco de transparencia que nos permite ver el efecto traslucido de las mismas\n" );
                ImGui::TextWrapped( "\nEn este menu se pueden controlar algunos aspectos generales del proyecto como el uso de proyeccion y modelo.\n" );
                ImGui::TextWrapped( "\nAl igual que manejaremos la direcciones de la luz y la ubicacion de la misma.\n" );

                if( ImGui::CollapsingHeader( "Iluminacion" ) ){

                    if( ImGui::TreeNode( "Luz direccional" ) ){
                        // mover posición
                        ImGui::SliderFloat3( "move_luz_direccional", &lightPosition.x, -10.0f, 10.0f );
                        ImGui::TreePop();
                    }

                    if( ImGui::TreeNode( "Punto de luz 1" ) ){
                        // mover posición
                        ImGui::SliderFloat3( "move_punto_luz_1", &pointLightPositions[ 0 ].x, -10.0f, 10.0f );
                        ImGui::TreePop();
                    }

                    if( ImGui::TreeNode( "Punto de luz 2" ) ){
                        // mover posición
                        ImGui::SliderFloat3( "move_punto_luz_2", &pointLightPositions[ 1 ].x, -10.0f, 10.0f );
                        ImGui::TreePop();
                    }

                    if( ImGui::TreeNode( "Punto de luz 3" ) ){
                        // mover posición
                        ImGui::SliderFloat3( "move_punto_luz_3", &pointLightPositions[ 2 ].x, -10.0f, 10.0f );
                        ImGui::TreePop();
                    }

                    if( ImGui::TreeNode( "Punto de luz 4" ) ){
                        // mover posición
                        ImGui::SliderFloat3( "move_punto_luz_4", &pointLightPositions[ 3 ].x, -10.0f, 10.0f );
                        ImGui::TreePop();
                    }

                }

                if( ImGui::CollapsingHeader( "Modelo" ) ){
                    // translate
                    ImGui::SliderFloat3( "Vector de Transacion: ", &translate_Vector.x, -10.0f, 10.0f );

                    //rotate
                    ImGui::SliderAngle( "Angulo de rotacion", &rotate_angle );
                    ImGui::SliderFloat3( "Vector de Rotacion", &rotate_Vector.x, 0.0f, 1.0f );

                    //scale
                    ImGui::SliderFloat3( "Escalado", &scale_Vector.x, -10.0f, 10.0f );
                }

                if( ImGui::CollapsingHeader( "Proyeccion" ) ){
                    ImGui::SliderFloat( "near", &near, -10.0f, 25.0f );
                    ImGui::SliderFloat( "far", &far, 30.0f, -150.0f );
                    ImGui::SliderFloat( "angulo o zoom", &camera.Zoom, 0.0, 50.0 );
                }

                if( ImGui::CollapsingHeader( "Animacion" ) ){
                    ImGui::Checkbox( "Detener animacion", &stop_animation );
                    ImGui::TextWrapped( "Aumentar el tamaño de la explocion en: " );
                    ImGui::InputFloat( "Escala:", &explotion_scale );
                }

                ImGui::Text( "Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate );

            }
            ImGui::End();

            ImGui::Render();
            ImGui_ImplGlfwGL3_RenderDrawData( ImGui::GetDrawData() );

            /* Swap front and back buffers */
            GLCall( glfwSwapBuffers( window ) );

            /* Poll for and process events */
            GLCall( glfwPollEvents() );

        }
    }

    // Cleanup
    ImGui_ImplGlfwGL3_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame andreact accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput( GLFWwindow *window ){
    if( glfwGetKey( window, GLFW_KEY_ESCAPE ) == GLFW_PRESS )
        glfwSetWindowShouldClose( window, true );
    if( glfwGetKey( window, GLFW_KEY_I ) == GLFW_PRESS ){
        GLCall( glPolygonMode( GL_FRONT_AND_BACK, GL_LINE ) );
        std::cout << "w press" << std::endl;
    }
    if( glfwGetKey( window, GLFW_KEY_O ) == GLFW_PRESS ){
        GLCall( glPolygonMode( GL_FRONT_AND_BACK, GL_FILL ) );
        std::cout << "S press" << std::endl;
    }
    if( glfwGetKey( window, GLFW_KEY_P ) == GLFW_PRESS ){
        GLCall( glPolygonMode( GL_FRONT_AND_BACK, GL_POINT ) );
        std::cout << "a press" << std::endl;
    }
    if( glfwGetKey( window, GLFW_KEY_W ) == GLFW_PRESS )
        camera.ProcessKeyboard( FORWARD, deltaTime );
    if( glfwGetKey( window, GLFW_KEY_S ) == GLFW_PRESS )
        camera.ProcessKeyboard( BACKWARD, deltaTime );
    if( glfwGetKey( window, GLFW_KEY_A ) == GLFW_PRESS )
        camera.ProcessKeyboard( LEFT, deltaTime );
    if( glfwGetKey( window, GLFW_KEY_D ) == GLFW_PRESS )
        camera.ProcessKeyboard( RIGHT, deltaTime );
    if( glfwGetKey( window, GLFW_KEY_Q ) == GLFW_PRESS ){
        // tell GLFW to capture our mouse
        glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_NORMAL );
        glfwSetCursorPosCallback( window, nullptr );
    }
    if( glfwGetKey( window, GLFW_KEY_E ) == GLFW_PRESS ){
        // tell GLFW to capture our mouse
        glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_DISABLED );
        firstMouse = true;
        glfwSetCursorPosCallback( window, mouse_callback );
        glfwSetScrollCallback( window, scroll_callback );
    }
}
// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback( GLFWwindow *window, int width, int height ){
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
    glViewport( 0, 0, width, height );
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback( GLFWwindow *window, double xpos, double ypos ){
    if( firstMouse ){
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement( xoffset, yoffset );
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback( GLFWwindow *window, double xoffset, double yoffset ){
    std::cout << xoffset << ", " << yoffset << std::endl;
    camera.Zoom -= ( float ) yoffset / 4;
    if( camera.Zoom < 1.0f )
        camera.Zoom = 1.0f;
    if( camera.Zoom > 50.0f )
        camera.Zoom = 50.0f;
    camera.ProcessMouseScroll( yoffset );
}