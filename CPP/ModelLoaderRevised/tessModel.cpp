
#include "tessModel.h"
#include "colorData.h"

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string>
#include <fstream>
#include <iostream>
#include <vector>

tessModel::tessModel( std::string inputFile ) {

    std::vector<GLuint> indices;
    std::vector<glm::vec4> vertices;

    // Default the model matrix to an identity matrix
    modelMat = glm::mat4( 1.0f );

    if( loadFromFile( inputFile, indices, vertices ) ) {
        
        glGenVertexArrays( 1, &VAOptr );
        glBindVertexArray( VAOptr );

            //////////////
            // Vertices //
            //////////////

            glGenBuffers( 1, &vertexBuffer );

            glBindBuffer( GL_ARRAY_BUFFER, vertexBuffer );
            glBufferData( GL_ARRAY_BUFFER, vertices.size() * sizeof( glm::vec4 ), 
                          &vertices[0][0], GL_STATIC_DRAW );

            // Vertex location
            glEnableVertexAttribArray( 0 );
            glVertexAttribPointer( 0, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0 );

            ////////////
            // Colors //
            ////////////

            glGenBuffers( 1, &colorBuffer );
            glBindBuffer( GL_ARRAY_BUFFER, colorBuffer );

            glBufferData( GL_ARRAY_BUFFER, sizeof( modelColor ), 
                          &modelColor, GL_STATIC_DRAW );
            
            // Ambient color location
            glEnableVertexAttribArray( 2 );
            glVertexAttribPointer( 2, 4, GL_FLOAT, GL_FALSE, 
                                   sizeof( modelColor ), 0 );
            glVertexAttribDivisor( 2, 1 );

            // Specular color location
            glEnableVertexAttribArray( 3 );
            glVertexAttribPointer( 3, 4, GL_FLOAT, GL_FALSE, sizeof( modelColor ), 
                                   (GLvoid*)( sizeof( glm::vec4 ) ) );
            glVertexAttribDivisor( 3, 1 );

            // Diffuse color location
            glEnableVertexAttribArray( 4 );
            glVertexAttribPointer( 4, 4, GL_FLOAT, GL_FALSE, sizeof( modelColor ), 
                                   (GLvoid*)( 2 * sizeof( glm::vec4 ) ) );            
            glVertexAttribDivisor( 4, 1 );

            // Shininess location
            glEnableVertexAttribArray( 5 );
            glVertexAttribPointer( 5, 1, GL_FLOAT, GL_FALSE, sizeof( modelColor ), 
                                   (GLvoid*)( 3 * sizeof( glm::vec4 ) ) );            
            glVertexAttribDivisor( 5, 1 );

            //////////////////
            // Model Matrix //
            //////////////////

            glGenBuffers( 1, &matrixBuffer );
            glBindBuffer( GL_ARRAY_BUFFER, matrixBuffer );

            glBufferData( GL_ARRAY_BUFFER, sizeof( glm::mat4 ), 
                          &modelMat[0][0], GL_STATIC_DRAW );

            int modelMatLoc = 6;            
            for( int i = 0; i < 4; i++ ) {

                glEnableVertexAttribArray( modelMatLoc + i );
                glVertexAttribPointer( modelMatLoc + i, 4, GL_FLOAT, GL_FALSE,
                                       sizeof( glm::mat4 ),
                                       (GLvoid*)( i * sizeof( glm::vec4 ) ) );
                glVertexAttribDivisor( modelMatLoc + i, 1 );
            }

            /////////////
            // Indices //
            /////////////

            // Indices of how the model is going to be drawn
            glGenBuffers( 1, &indicesVBOptr );
            glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, indicesVBOptr );
            glBufferData( GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof( GLuint ),
                          &indices[0], GL_STATIC_DRAW );

        glBindVertexArray( 0 );

        numIndices = indices.size();
    }
    else {
        std::cout << "Something exploded!" << std::endl;
    }
}

tessModel::~tessModel() {
    glDeleteVertexArrays( 1, &VAOptr );
    glDeleteBuffers( 1, &vertexBuffer );
    glDeleteBuffers( 1, &colorBuffer );
    glDeleteBuffers( 1, &matrixBuffer );
    glDeleteBuffers( 1, &indicesVBOptr );
}

void tessModel::draw() {

    glBindVertexArray( VAOptr );

        glPatchParameteri( GL_PATCH_VERTICES, 16 );
        glDrawElementsInstanced( GL_PATCHES, numIndices, GL_UNSIGNED_INT, 0, 1 );

    glBindVertexArray( 0 );
}

void tessModel::draw( GLuint numElements ) {

    glBindVertexArray( VAOptr );

        glPatchParameteri( GL_PATCH_VERTICES, 16 );
        glDrawElementsInstanced( GL_PATCHES, numElements, GL_UNSIGNED_INT, 0, 1 );

    glBindVertexArray( 0 );
}

void tessModel::drawCtrlPts( GLenum primitive ) {

    glBindVertexArray( VAOptr );

        glDrawElementsInstanced( primitive, numIndices, GL_UNSIGNED_INT, 0, 1 );

    glBindVertexArray( 0 );
}

void tessModel::setColor( colorData color ) {
    modelColor = color;
    updateColor();
}

void tessModel::setAmbColor( glm::vec4 amb ) {
    modelColor.amb = amb;
    updateColor();
}

void tessModel::setSpecColor( glm::vec4 spec ) {
    modelColor.spec = spec;
    updateColor();
}

void tessModel::setDiffColor( glm::vec4 diff ) {
    modelColor.diff = diff;
    updateColor();
}

void tessModel::setShininess( float shininess )  {
    modelColor.shininess = shininess;
    updateColor();
}

bool tessModel::loadFromFile( std::string inputFile,
                              std::vector<GLuint>& indices,
                              std::vector<glm::vec4>& vertices ) {

    std::ifstream fin( inputFile.c_str() );
    if( fin.fail() ) {
        std::cout << "Error! Failed to open " << inputFile << "!" << std::endl;
        return false;
    }
    else {

        // Get all the indices that'll be put together as control points
        // For patches. They come in sets of 16
        int numPatches;
        fin >> numPatches;
        fin.ignore();
        for( int i = 0; i < numPatches; i++ ) {
            std::string input;
            for( int i = 0; i < 16 - 1; i++ ) {
                getline( fin, input, ',' );
                indices.push_back( atoi ( input.c_str() ) - 1 );
            }
            getline( fin, input );
            indices.push_back( atoi( input.c_str() ) - 1 );

        }

        int numVertices;
        fin >> numVertices;
        //std::cout << "Number of vertices: " << numVertices << std::endl;
        
        for( int i = 0; i < numVertices; i++ ) {
            std::string input;
            glm::vec4 vertex = glm::vec4( 0.0, 0.0, 0.0, 1.0 );
            for( int i = 0; i < 2; i++ ) {
                getline( fin, input, ',' );
                vertex[i] = atof( input.c_str() );
            }
            getline( fin, input );
            vertex[2] = atof( input.c_str() );
            vertices.push_back( vertex );
        }

        fin.close();

    }
    return true;
}


void tessModel::updateColor() {
    glBindBuffer( GL_ARRAY_BUFFER, colorBuffer );

    glBufferData( GL_ARRAY_BUFFER, sizeof( modelColor ), 
                          &modelColor, GL_STATIC_DRAW );

    glBindBuffer( GL_ARRAY_BUFFER, 0 );
}