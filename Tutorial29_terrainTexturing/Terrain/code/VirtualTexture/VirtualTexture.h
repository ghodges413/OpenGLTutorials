//
//  VirtualTexture.h
//
#pragma once
#include "Math/Vector.h"
#include "Graphics/Mesh.h"
#include "Graphics/RenderSurface.h"
#include "Graphics/Shader.h"
#include "Miscellaneous/Types.h"
#include "VirtualTexture/Common.h"
#include "VirtualTexture/PageTable.h"

/*
================================
VirtualTexture

This is the public facing interface,
as well as the general manager of the
virtual texturing system
================================
*/
class VirtualTexture {
public:
	VirtualTexture();
	~VirtualTexture() { Cleanup(); }

	void Init();
	void Cleanup();
	void InitSamplerFeedback( int width, int height );

	// The feedback pass can be significantly smaller than the final render (up to 10x smaller)
	void BeginSamplerFeedBack();
	void EndSamplerFeedBack();
	Shader * GetFeedbackShader() { return &m_feedbackShader; }
	unsigned int GetSamplerFeedbackFBO() const { return m_feedbackSurface.GetFBO(); }

	void BeginMegaTextureDraw();
	Shader * GetMegatextureShader() { return &m_megatextureShader; }

	unsigned int GetPhysicalTexture() const { return m_pageTable.GetPhysicalTexture(); }

private:
	PageTable m_pageTable;

	Shader m_megatextureShader;	// Shader used for drawing geo w/ the megatexture
	Shader m_feedbackShader;	// Shader used to fill the sampler feedback buffer
	

	// Feedback related
	PixelFeedback_t * m_feedbackBuffer;
	int m_width;
	int m_height;

	// GL framebuffer the page id pass is rendered to.
	// This framebuffer is usually much smaller than the screen resolution.
	RenderSurface m_feedbackSurface;	// The fbo to render the sampler feedback into
};