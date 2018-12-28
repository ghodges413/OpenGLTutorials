/*
 *  VertexBufferObject.h
 *
 */
#pragma once


/*
 ======================================
 VertexBufferObject
 ======================================
 */
class VertexBufferObject {
private:
    VertexBufferObject( const VertexBufferObject & rhs );
    const VertexBufferObject & operator = ( const VertexBufferObject & rhs );
    
public:
    VertexBufferObject();
    ~VertexBufferObject();
	void Cleanup();
    
    void Generate( const unsigned int type, const unsigned long dataSize, const void * const data_ptr, const unsigned int hint );
    void Update( const void * data_ptr ) const;
    bool IsValid() const { return ( mVBO > 0 ); }
    
    void Bind() const;
    void UnBind() const;
    
	unsigned int GetVBO() const { return mVBO; }
    unsigned int GetType() const { return mType; }
    unsigned int GetHint() const { return mHint; }
    
private:
    unsigned int mVBO;
    unsigned int mType; // GL_ARRAY_BUFFER or GL_ELEMENT_ARRAY_BUFFER
    unsigned int mHint; // GL_STATIC_DRAW GL_DYNAMIC_DRAW or GL_STREAM_DRAW
    unsigned long mDataSize;
};

