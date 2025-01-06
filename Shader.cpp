//
//  Shader.cpp
//  Lab3
//
//  Created by CGIS on 05/10/2016.
//  Copyright © 2016 CGIS. All rights reserved.
//

#include "Shader.hpp"

namespace gps {
    std::string Shader::readShaderFile(std::string fileName) {
        std::ifstream shaderFile;
        std::string shaderString;

        // Open shader file
        shaderFile.open(fileName);

        if (!shaderFile.is_open()) {
            std::cerr << "Failed to open shader file: " << fileName << std::endl;
            return "";
        }

        std::stringstream shaderStringStream;

        // Read shader content into stream
        shaderStringStream << shaderFile.rdbuf();

        // Close shader file
        shaderFile.close();

        // Convert stream into string
        shaderString = shaderStringStream.str();
        return shaderString;
    }

    void Shader::shaderCompileLog(GLuint shaderId) {
        GLint success;
        GLchar infoLog[512];

        // Check compilation info
        glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);

        if (!success) {
            glGetShaderInfoLog(shaderId, 512, NULL, infoLog);
            std::cout << "Shader compilation error:\n" << infoLog << std::endl;
        }
    }

    void Shader::shaderLinkLog(GLuint shaderProgramId) {
        GLint success;
        GLchar infoLog[512];

        // Check linking info
        glGetProgramiv(shaderProgramId, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shaderProgramId, 512, NULL, infoLog);
            std::cout << "Shader linking error:\n" << infoLog << std::endl;
        }
    }

    void Shader::loadShader(std::string vertexShaderFileName, std::string fragmentShaderFileName, std::string geometryShaderFileName) {
        // Read, parse, and compile the vertex shader
        std::string v = readShaderFile(vertexShaderFileName);
        const GLchar* vertexShaderString = v.c_str();
        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderString, NULL);
        glCompileShader(vertexShader);
        shaderCompileLog(vertexShader);

        // Read, parse, and compile the fragment shader
        std::string f = readShaderFile(fragmentShaderFileName);
        const GLchar* fragmentShaderString = f.c_str();
        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderString, NULL);
        glCompileShader(fragmentShader);
        shaderCompileLog(fragmentShader);

        // Optional: Read, parse, and compile the geometry shader
        GLuint geometryShader = 0;
        if (!geometryShaderFileName.empty()) {
            std::string g = readShaderFile(geometryShaderFileName);
            const GLchar* geometryShaderString = g.c_str();
            geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
            glShaderSource(geometryShader, 1, &geometryShaderString, NULL);
            glCompileShader(geometryShader);
            shaderCompileLog(geometryShader);
        }

        // Attach and link the shader programs
        this->shaderProgram = glCreateProgram();
        glAttachShader(this->shaderProgram, vertexShader);
        glAttachShader(this->shaderProgram, fragmentShader);
        if (geometryShader != 0) {
            glAttachShader(this->shaderProgram, geometryShader);
        }
        glLinkProgram(this->shaderProgram);

        // Clean up shaders (they are linked into the program now)
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        if (geometryShader != 0) {
            glDeleteShader(geometryShader);
        }

        // Check linking info
        shaderLinkLog(this->shaderProgram);
    }

    void Shader::useShaderProgram() {
        glUseProgram(this->shaderProgram);
    }


}
