//
//  Input.h
//
#pragma once

#include "Math/Vector.h"

/*
 =====================================
 Keyboard
 =====================================
 */
class Keyboard {
public:
	enum KeyboardButtons_t {
		kb_BACKSPACE = 8,
		kb_ENTER = 13,
		kb_SHIFT_OUT = 14,
		kb_SHIFT_IN = 15,
		kb_ESC = 27,
        kb_SPACE = 32,
		kb_DEL = 127,
	};
public:
    Keyboard();
    ~Keyboard();
    
    const bool      IsKeyDown( unsigned char key ) const    { return m_keysDown[ key ]; }
    bool            IsKeyDown( unsigned char key )          { return m_keysDown[ key ]; }
    
    void            SetKeyDown( unsigned char key );
    void            SetKeyUp( unsigned char key );
    
    bool            WasKeyDown( unsigned char key );
    
private:
    static const int s_maxKeys = 256;
    bool    m_keysDown[ s_maxKeys ];
    bool    m_wasKeysDown[ s_maxKeys ];
};

/*
 =====================================
 Mouse
 =====================================
 */
class Mouse {
public:
	enum MouseButton_t {
		mb_left = 0,
		mb_right = 1,
		mb_middle = 2,
		mb_thumb = 3,
		mb_ring = 4,
	};
public:
    Mouse();
	~Mouse() {}

	Vec2d GLPos() const { return Vec2d( m_pos.x, m_pos.y ); }
	const float & ScrollPos() const { return m_pos.z; }

	Vec3d GLPosDelta() { Vec3d delta = ( m_pos - m_prevPos ); m_prevPos = m_pos; return delta; }
	
	void SetPos( const Vec3d & pos ) { m_prevPos = m_pos; m_pos = pos; }

	bool IsButtonDown( const MouseButton_t & idx ) { /*assert( idx >= 0 && idx < sMaxButtons );*/ return m_buttonsDown[ idx ]; }
	bool WasButtonDown( const MouseButton_t & idx );

	void SetButtonState( const MouseButton_t & idx, const bool & isDown );

private:
	Vec3d m_pos;	// in gl screen space, not windows screen space
	Vec3d m_prevPos;

	static const int s_maxButtons = 5;
	bool m_buttonsDown[ s_maxButtons ];	// Up to 5 mouse buttons
	bool m_wasButtonsDown[ s_maxButtons ];
};

extern Keyboard g_keyboard;
extern Mouse g_mouse;




void keyboard( unsigned char key, int x, int y );
void keyboardup( unsigned char key, int x, int y );
void special( int key, int x, int y );
void mouse( int button, int state, int x, int y );
void motion( int x, int y );
void motionPassive( int x, int y );
void entry( int state );