//
//  main.cpp
//  OpenGL Advances Lighting
//
//  Created by CGIS on 28/11/16.
//  Copyright � 2016 CGIS. All rights reserved.
//

#if defined (__APPLE__)
#define GLFW_INCLUDE_GLCOREARB
#define GL_SILENCE_DEPRECATION
#else
#define GLEW_STATIC
#include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.hpp"
#include "Model3D.hpp"
#include "Camera.hpp"
#include "SkyBox.hpp"

#include <iostream>

int glWindowWidth = 2048;
int glWindowHeight = 1024;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;

const unsigned int SHADOW_WIDTH = 2048;
const unsigned int SHADOW_HEIGHT = 2048;

struct Keyframe {
    glm::vec3 target;
    glm::vec3 position;
    float time; // Time in seconds
};

std::vector<Keyframe> keyframes = {
        {glm::vec3(0.639806f, -0.42578f, -0.639812f), glm::vec3(-5.94317f, 3.80591f, 6.05123f),0.0f},
        {glm::vec3(0.64594f, -0.446198f, 0.619411f), glm::vec3(-6.69056f, 3.80591f, -5.76073f),4.0f},
        {glm::vec3(-0.615539f, -0.335452f, 0.71315f), glm::vec3(6.23018f, 3.6061f, -6.80397f),8.0f},
        {glm::vec3(-0.663713f, -0.403546f, -0.62979f), glm::vec3(6.36945f, 3.6061f, 6.10475f),12.0f},
        {glm::vec3(-0.26511f, -0.16849f, -0.949383f), glm::vec3(1.62708f, 1.00399f, 3.54295f),15.5f},
        {glm::vec3(-0.144936f, -0.127065f, -0.981248f), glm::vec3(0.721653f, 0.640923f, 0.193045f),18.5f},
        {glm::vec3(-0.915322f, -0.0279219f, -0.401753f), glm::vec3(-0.713158f, 0.534659f, -1.00396f),22.0f},
        {glm::vec3(-0.338687f, -0.00698148f, 0.940873f), glm::vec3(-2.11822f, 0.537763f, -1.70441f),25.0f},
        {glm::vec3(0.741273f, 0.0714972f, 0.667385f), glm::vec3(-2.41473f, 0.509165f, -1.97137f),28.0f},

};

float animationTime = 0.0f; // Current time in the animation
int currentKeyframe = 0;
bool cinematicMode;
float lastFrameTime = 0.0f; // Time of the last frame
float delta_time = 0.0f;
float wingRotationAngle = 0.0f;
const float rotationSpeed = 50.0f;
float frameTime=-2.0f;
float timeIncrement=0;


glm::mat4 model;
GLuint modelLoc;
glm::mat4 view;
GLuint viewLoc;
glm::mat4 projection;
GLuint projectionLoc;
glm::mat3 normalMatrix;
GLuint normalMatrixLoc;
glm::mat4 lightRotation;

glm::vec3 lightDir;
GLuint lightDirLoc;
glm::vec3 lightColor;
GLuint lightColorLoc;

gps::Camera myCamera(
	glm::vec3(0.0f, 1.0f, 1.5f),
	glm::vec3(0.0f, 0.0f, 1.0f),
	glm::vec3(0.0f, 1.0f, 0.0f));
float cameraSpeed = 0.03f;

bool pressedKeys[1024];
float angleY = 0.0f;
GLfloat lightAngle;

GLfloat ambientStrength = 0.7f;
GLfloat specularStrength = 0.5f;
GLfloat shininess = 32.0f;
GLboolean showFog = false;
GLboolean pointShadows = false;

gps::Model3D scene;
gps::Model3D wings;
gps::Model3D lightCube;
gps::Model3D screenQuad;

gps::Shader myCustomShader;
gps::Shader lightShader;
gps::Shader screenQuadShader;
gps::Shader depthMapShader;
gps::Shader pointLightShader;

GLuint shadowMapFBO;
GLuint depthMapTexture;
GLuint depthMapFBO;
GLuint depthCubeMap;

bool showDepthMap;
bool rotateWings;

//Skybox stuff
GLuint textureSkyboxID;
gps::SkyBox mySkyBox;
gps::Shader skyboxShader;

void initSkybox() {
	glGenTextures(1, &textureSkyboxID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureSkyboxID);

	std::vector<const GLchar*> faces;
	faces.push_back("../skybox/right.tga");
	faces.push_back("../skybox/left.tga");
	faces.push_back("../skybox/top.tga");
	faces.push_back("../skybox/bottom.tga");
	faces.push_back("../skybox/back.tga");
	faces.push_back("../skybox/front.tga");
	mySkyBox.Load(faces);

	skyboxShader.loadShader("../shaders/skyboxShader.vert", "../shaders/skyboxShader.frag");
	skyboxShader.useShaderProgram();
}


GLenum glCheckError_(const char* file, int line) {
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
    fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);

    glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

    myCustomShader.useShaderProgram();

    //set projection matrix
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
    //send matrix data to shader
    GLint projLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    lightShader.useShaderProgram();

    glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    //set Viewport transform
    glViewport(0, 0, retina_width, retina_height);
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_M && action == GLFW_PRESS)
		showDepthMap = !showDepthMap;

    if (key == GLFW_KEY_F && action == GLFW_PRESS)
        showFog = !showFog;

    if(key == GLFW_KEY_C && action == GLFW_PRESS){
        cinematicMode = true;
        animationTime = 0.0f;
        currentKeyframe = 0;
    }

    if (key == GLFW_KEY_LEFT_ALT && action == GLFW_PRESS){
        timeIncrement += 0.5f;
    }

    if (key == GLFW_KEY_LEFT_CONTROL && action == GLFW_PRESS){
        timeIncrement -= 0.5f;
    }

    if(key == GLFW_KEY_G && action == GLFW_PRESS){
        frameTime += 2.0f + timeIncrement;
        std::cout <<"{glm::vec3(" <<myCamera.getCameraDirection().x << "f, " << myCamera.getCameraDirection().y << "f, " << myCamera.getCameraDirection().z << "f), ";
        std::cout << "glm::vec3(" <<myCamera.getCameraPosition().x << "f, " << myCamera.getCameraPosition().y << "f, " << myCamera.getCameraPosition().z <<"f)," << frameTime<<"f}," <<std::endl;
    }

    if (key == GLFW_KEY_Z && action == GLFW_PRESS){
        pointShadows = !pointShadows;
        glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "pointShadows"), pointShadows);
    }

    if(pressedKeys[GLFW_KEY_R]){
        rotateWings = !rotateWings;
    }

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			pressedKeys[key] = true;
		else if (action == GLFW_RELEASE)
			pressedKeys[key] = false;
	}
}

float lastX;
float lastY;
bool firstMouse = true;

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.3f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	static float yaw = -90.0f;
	static float pitch = 0.0f;
	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0f) pitch = 89.0f;
	if (pitch < -89.0f) pitch = -89.0f;

	myCamera.rotate(pitch, yaw);

}

void processMovement()
{
	if (pressedKeys[GLFW_KEY_Q]) {
		angleY -= 1.0f;
	}

	if (pressedKeys[GLFW_KEY_E]) {
		angleY += 1.0f;
	}

	if (pressedKeys[GLFW_KEY_J]) {
		lightAngle -= 1.0f;
	}

	if (pressedKeys[GLFW_KEY_L]) {
		lightAngle += 1.0f;
	}

	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
	}
    if (pressedKeys[GLFW_KEY_B]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    }
    if (pressedKeys[GLFW_KEY_N]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    if (pressedKeys[GLFW_KEY_V]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    }

    if (pressedKeys[GLFW_KEY_LEFT_SHIFT])
    {
        cameraSpeed = 0.3f;
    }
    else
    {
        cameraSpeed = 0.02f;
    }

    if (pressedKeys[GLFW_KEY_UP])
    {
        ambientStrength += 0.005f;
        specularStrength +=0.005f;
        myCustomShader.useShaderProgram();
        glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "ambientStrength"), ambientStrength);
        glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "specularStrength"), specularStrength);
    }

    if (pressedKeys[GLFW_KEY_DOWN])
    {
        ambientStrength -= 0.005f;
        specularStrength -= 0.005f;
        myCustomShader.useShaderProgram();
        glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "ambientStrength"), ambientStrength);
        glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "specularStrength"), specularStrength);
    }

}

bool initOpenGLWindow()
{
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//window scaling for HiDPI displays
	glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
	//for sRBG framebuffer
	glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);
	//for antialising
	glfwWindowHint(GLFW_SAMPLES, 4);
	glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "OpenGL Shader Example", NULL, NULL);
	if (!glWindow) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}
	glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
	glfwSetKeyCallback(glWindow, keyboardCallback);
	glfwSetCursorPosCallback(glWindow, mouseCallback);
	//glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwMakeContextCurrent(glWindow);
	glfwSwapInterval(1);

#if not defined (__APPLE__)
	// start GLEW extension handler
	glewExperimental = GL_TRUE;
	glewInit();
#endif

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION); // version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	return true;
}

glm::vec3 interpolate(const glm::vec3& start, const glm::vec3& end, float t) {
    // Ease-in/ease-out interpolation
    float smoothT = t * t * (3.0f - 2.0f * t); // Smoothstep function
    return glm::mix(start, end, smoothT);
}

glm::vec3 catmullRom(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, float t) {
    float t2 = t * t;
    float t3 = t2 * t;

    return 0.5f * (
            (2.0f * p1) +
            (-p0 + p2) * t +
            (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t2 +
            (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t3
    );
}


void processCinematicMovement(float deltaTime) {
    if (!cinematicMode || keyframes.empty()) return;

    animationTime += deltaTime;

    if (currentKeyframe < keyframes.size() - 1) {
        Keyframe& kf1 = keyframes[currentKeyframe];
        Keyframe& kf2 = keyframes[currentKeyframe + 1];

        // Determine adjacent keyframes for splines
        glm::vec3 p0 = currentKeyframe > 0 ? keyframes[currentKeyframe - 1].position : kf1.position;
        glm::vec3 p3 = currentKeyframe + 2 < keyframes.size() ? keyframes[currentKeyframe + 2].position : kf2.position;

        // Check if it's time to move to the next keyframe
        if (animationTime > kf2.time) {
            currentKeyframe++;
            if (currentKeyframe >= keyframes.size() - 1) {
                cinematicMode = false; // End cinematic mode
                return;
            }
        }

        // Interpolation factor
        float t = (animationTime - kf1.time) / (kf2.time - kf1.time);
        t = glm::clamp(t, 0.0f, 1.0f);

        // Interpolate using Catmull-Rom
        glm::vec3 interpolatedPosition = catmullRom(p0, kf1.position, kf2.position, p3, t);
        glm::vec3 interpolatedTarget = interpolate(kf1.target, kf2.target, t); // Smoothstep for target

        // Update the camera
        myCamera.setPosition(interpolatedPosition);
        myCamera.setTarget(interpolatedTarget);
    }
}


void initOpenGLState()
{
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	glViewport(0, 0, retina_width, retina_height);

	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise

	glEnable(GL_FRAMEBUFFER_SRGB);
}

void initShaders() {
	myCustomShader.loadShader("../shaders/shaderStart.vert", "../shaders/shaderStart.frag");
	myCustomShader.useShaderProgram();
	lightShader.loadShader("../shaders/lightCube.vert", "../shaders/lightCube.frag");
	lightShader.useShaderProgram();
	screenQuadShader.loadShader("../shaders/screenQuad.vert", "../shaders/screenQuad.frag");
	screenQuadShader.useShaderProgram();
	depthMapShader.loadShader("../shaders/depthMapShader.vert", "../shaders/depthMapShader.frag");
	depthMapShader.useShaderProgram();
    pointLightShader.loadShader("../shaders/pointShadow.vert", "../shaders/pointShadow.frag", "../shaders/pointShadow.geom");
    pointLightShader.useShaderProgram();
}

void initUniforms() {
	myCustomShader.useShaderProgram();

	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//set the light direction (direction towards the light)
	lightDir = glm::vec3(1.5f, 2.0f, 0.0f);
	lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	lightDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDir");
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

	//set light color
	lightColor = glm::vec3(1.0f, 0.5f, 0.4f);
	lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

	lightShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
}

void initObjects() {
    scene.LoadModel("../objects/scene/scene.obj");
    wings.LoadModel("../objects/scene/wings.obj");
    lightCube.LoadModel("../objects/sun/sun.obj");
    screenQuad.LoadModel("../objects/quad/quad.obj");
}

void initFBO() {

	glGenFramebuffers(1, &shadowMapFBO);
	glGenTextures(1, &depthMapTexture);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	//attach texture to FBO
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture,
		0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


    // point light shadow


    glGenFramebuffers(1, &depthMapFBO);
    glGenTextures(1, &depthCubeMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubeMap);

    for (unsigned int i = 0; i < 6; ++i) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
                     SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    // Attach cube map to FBO
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubeMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

glm::mat4 computeLightSpaceTrMatrix() {
    glm::mat4 lightView = glm::lookAt(glm::vec3(0.0, 4.0, -4.0), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    const GLfloat near_plane = 1.0f, far_plane = 20.5f;
    glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
    glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;

    return lightSpaceTrMatrix;
}

void renderObjects(gps::Shader shader, bool depthPass) {
    //scene
    shader.useShaderProgram();
    model = glm::rotate(glm::mat4(1.0f), glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
    //model = glm::scale(model, glm::vec3(0.2f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    scene.Draw(shader);

    //wings
    shader.useShaderProgram();
    float initialRotation = 142.8;
    glm::vec3 wingCenter = glm::vec3(2.0435f, -1.4229f, 1.8038f);

    model = glm::mat4(1.0f);
    model = glm::rotate(glm::mat4(1.0f), glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::translate(model, wingCenter);
    model = glm::rotate(model, glm::radians(initialRotation), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::rotate(model, glm::radians(wingRotationAngle), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(-initialRotation), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::translate(model, -wingCenter);
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    wings.Draw(shader);

}

float farPlane = 25.0f;
void renderDepthMapPointLight(glm::vec3 lightPos) {
    float nearPlane = 1.0f;
    pointLightShader.useShaderProgram();

    glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, nearPlane, farPlane);
    std::vector<glm::mat4> shadowTransforms;
    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f)));

    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    pointLightShader.useShaderProgram();
    for (unsigned int i = 0; i < 6; ++i) {
        std::string name = "shadowMatrices[" + std::to_string(i) + "]";
        glUniformMatrix4fv(glGetUniformLocation(pointLightShader.shaderProgram, name.data()), 1, GL_FALSE, glm::value_ptr(shadowTransforms[i]));
    }
    glUniform1f(glGetUniformLocation(pointLightShader.shaderProgram, "far_plane"), farPlane);
    glUniform3fv(glGetUniformLocation(pointLightShader.shaderProgram, "lightPos"), 1, glm::value_ptr(lightPos));

    renderObjects(pointLightShader, true);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void renderDepthMapDirectionalLight(){
    depthMapShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"),1,GL_FALSE,glm::value_ptr(computeLightSpaceTrMatrix()));

    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    //RenderTheScene();
    renderObjects(depthMapShader, true);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void renderScene() {

    lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));

    glm::mat4 pointLight = glm::mat4(1.0f);
    pointLight = glm::scale(pointLight, glm::vec3(2.0f, 2.0f, 2.0f));
    pointLight *= lightRotation;
    pointLight = glm::translate(pointLight, 1.0f * lightDir);

    const glm::vec3 lightPos = glm::vec3(pointLight[3]);

	//render the scene to the depth buffer
    renderDepthMapDirectionalLight();

    if(pointShadows)
        renderDepthMapPointLight(lightPos);

	// render depth map on screen - toggled with the M key
	if (showDepthMap) {
		glViewport(0, 0, retina_width, retina_height);
		glClear(GL_COLOR_BUFFER_BIT);
		screenQuadShader.useShaderProgram();

		//bind the depth map
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(screenQuadShader.shaderProgram, "depthMap"), 0);

		glDisable(GL_DEPTH_TEST);
		screenQuad.Draw(screenQuadShader);
		glEnable(GL_DEPTH_TEST);
	}
	else {
		// final scene rendering pass (with shadows)
		glViewport(0, 0, retina_width, retina_height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		myCustomShader.useShaderProgram();

        glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "showFog"), showFog);

		glActiveTexture(GL_TEXTURE0 + 5);
		glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "skybox"), 5);
		glBindTexture(GL_TEXTURE_CUBE_MAP, mySkyBox.GetTextureId());

		view = myCamera.getViewMatrix();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

		//bind the shadow map
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 3);
		glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"),
			1,
			GL_FALSE,
			glm::value_ptr(computeLightSpaceTrMatrix()));
        if(pointShadows) {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubeMap);
            glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "pointShadowMap"), 1);
            glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightPos"), 1, glm::value_ptr(lightPos));
            glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "far_plane"), farPlane);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubeMap);
        }

		//draw a white cube around the light
		lightShader.useShaderProgram();
        glUniform3fv(glGetUniformLocation(lightShader.shaderProgram, "cameraPosWorld"), 1, glm::value_ptr(myCamera.cameraPosition));
		glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(pointLight));
		lightCube.Draw(lightShader);

		// draw the Skybox
		mySkyBox.Draw(skyboxShader, view, projection);

        renderObjects(myCustomShader, false);

	}
}

void cleanup() {
	glDeleteTextures(1, &depthMapTexture);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteTextures(1, &depthCubeMap);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &shadowMapFBO);
    glDeleteFramebuffers(1, &depthMapFBO);
	glfwDestroyWindow(glWindow);
	//close GL context and any other GLFW resources
	glfwTerminate();
}

int main(int argc, const char* argv[]) {

	if (!initOpenGLWindow()) {
		glfwTerminate();
		return 1;
	}

	initOpenGLState();
	initObjects();
	initShaders();
	initUniforms();
	initFBO();
	initSkybox();

	glCheckError();

	while (!glfwWindowShouldClose(glWindow)) {
        float currentFrameTime = glfwGetTime();
        delta_time = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;

        if(rotateWings){
            wingRotationAngle += rotationSpeed * delta_time;
            if (wingRotationAngle >= 360.0f) wingRotationAngle -= 360.0f;
        }

		processMovement();
        processCinematicMovement(delta_time);
		renderScene();

		glfwPollEvents();
		glfwSwapBuffers(glWindow);
	}

	cleanup();

	return 0;
}
