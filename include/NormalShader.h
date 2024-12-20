struct NormalShader : Shader {
    glm::mat4 modelview = glm::mat4(1.0f);
    GLuint modelview_loc = 0;
    glm::mat4 projection = glm::mat4(1.0f);
    GLuint projection_loc = 0;
    GLuint isHighlighted_loc = 0;

    NormalShader() {}

    void initUniforms() {
        modelview_loc = glGetUniformLocation(program, "modelview");
        projection_loc = glGetUniformLocation(program, "projection");
        isHighlighted_loc = glGetUniformLocation(program, "isHighlighted");
    }

    void setUniforms(bool isHighlighted = false) {
        glUniformMatrix4fv(modelview_loc, 1, GL_FALSE, &modelview[0][0]);
        glUniformMatrix4fv(projection_loc, 1, GL_FALSE, &projection[0][0]);
        glUniform1i(isHighlighted_loc, static_cast<GLint>(isHighlighted));
    }
};