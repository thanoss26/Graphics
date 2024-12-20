/**************************************************
Geometry is an abstract class for a geometric object.
 A derived class of Geometry will have an implemention of
   ```void init(void)```
 or
   ```void init(const char* s).```
 The option for const char* input is for a derived class
 that requires reading external files.
 The implementation of the `init` is for initializing the
 geometry (i.e. setting up the buffers etc).
 
 The draw command is fixed.  We can call
 
 glBindVertexArray(obj.vao);
 glDrawElements(obj.mode, obj.count, obj.type, 0);
 
which should explain the purpose of those class members.
 We can also just call the "draw()" member function, which
 is equivalent to the commands above.
 
The array of buffers is encapsulated in std::vector so
we do not need to manually allocate/free the memory for
arrays of unknown size.
*****************************************************/
#include <vector>
#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>


#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__

class Geometry {
public:
    GLenum mode = GL_TRIANGLES; // the cookboook for glDrawElements
    int count; // number of elements to draw
    GLenum type = GL_UNSIGNED_INT; // type of the index array
    GLuint vao; // vertex array object a.k.a. geometry spreadsheet
    std::vector<GLuint> buffers; // data storage
    
    virtual void init(){};
    virtual void init(const char* s){};
    
    glm::mat4 model = glm::mat4(1.0f);   // model matrix


    // Model translation
    void translate(const float tx, const float ty, const float tz) {
        model = glm::translate(model, glm::vec3(tx, ty, tz));
    }
    
    void rotateX(const float degrees) {
        model = glm::rotate(model, glm::radians(degrees), glm::vec3(1.0f, 0.0f, 0.0f));
    }

    void rotateY(const float degrees) {
        model = glm::rotate(model, glm::radians(degrees), glm::vec3(0.0f, 1.0f, 0.0f));
    }

    void rotateZ(const float degrees) {
        model = glm::rotate(model, glm::radians(degrees), glm::vec3(0.0f, 0.0f, 1.0f));
    }


    void reset(void) {
        model = glm::mat4(1.0f);
    }


    void draw(void){
        glBindVertexArray(vao);
        glDrawElements(mode,count,type,0);
    }
};

#endif 
