/*
 *  ShaderStorageBuffer.h
 *
 */
#pragma once

/*
 ======================================
 ShaderStorageBuffer
 ======================================
 */
class ShaderStorageBuffer {
private:
    ShaderStorageBuffer( const ShaderStorageBuffer & rhs );
    const ShaderStorageBuffer & operator = ( const ShaderStorageBuffer & rhs );
    
public:
    ShaderStorageBuffer();
    ~ShaderStorageBuffer();
    
	bool	Generate( const unsigned int structureSize, const unsigned int structureCount, const int flags );

    bool	Generate( const unsigned int structureSize, const unsigned int structureCount );
    bool	IsValid() const { return ( mBufferObject > 0 ); }

	void *	MapBuffer( const int bufferMask );
    
#ifdef WINDOWS
	void	UnMapBuffer();

	void	Bind( const unsigned int index );
	void	Bind();
#else
    void	UnMapBuffer() {}
    
	void	Bind( const unsigned int index ) {}
	void	Bind() {}
#endif
    
private:
    unsigned int mBufferObject;		// The GL id
	unsigned int mStructureSize;	// The size of the individual data structure being stored
	unsigned int mStructureCount;	// The maximum number of structures

	static const int sNumBuffers = 1;
};

