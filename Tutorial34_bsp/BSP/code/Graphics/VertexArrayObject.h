//
//  VertexArrayObject
//
#pragma once

/*
 ======================================
 VertexArrayObject
 ======================================
 */
class VertexArrayObject {
private:
    VertexArrayObject( const VertexArrayObject & rhs );
    const VertexArrayObject & operator = ( const VertexArrayObject & rhs );
    
public:
    VertexArrayObject();
    ~VertexArrayObject();
	void Cleanup();
    
    void Generate();
    bool IsValid() const { return ( mVAO > 0 ); }
    
    void Bind() const;
    void UnBind() const;
private:
    unsigned int mVAO;

	// The currently bound VAO
	static unsigned int sBoundVAO;
};

