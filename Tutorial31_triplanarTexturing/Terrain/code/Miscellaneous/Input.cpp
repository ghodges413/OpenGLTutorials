//
//  Input.cpp
//
#include "Miscellaneous/Input.h"

Keyboard g_keyboard;
Mouse g_mouse;

/*
 ==========================================================================
 
 Keyboard

 ==========================================================================
 */

/*
 =====================================
 Keyboard::Keyboard
 =====================================
 */
Keyboard::Keyboard() {
    for ( int i = 0; i < s_maxKeys; ++i ) {
        m_keysDown[ i ] = false;
        m_wasKeysDown[ i ] = false;
    }
}

/*
 =====================================
 Keyboard::~Keyboard
 =====================================
 */
Keyboard::~Keyboard() {
    for ( int i = 0; i < s_maxKeys; ++i ) {
        m_keysDown[ i ] = false;
        m_wasKeysDown[ i ] = false;
    }
}

/*
 =====================================
 Keyboard::SetKeyDown
 =====================================
 */
void Keyboard::SetKeyDown( unsigned char key ) {
    m_keysDown[ key ] = true;
    m_wasKeysDown[ key ] = true;
}

/*
 =====================================
 Keyboard::SetKeyUp
 =====================================
 */
void Keyboard::SetKeyUp( unsigned char key ) {
    m_keysDown[ key ] = false;
}

/*
 =====================================
 Keyboard::WasKeyDown
 =====================================
 */
bool Keyboard::WasKeyDown( unsigned char key ) {
    if ( m_wasKeysDown[ key ] && !m_keysDown[ key ] ) {
        m_wasKeysDown[ key ] = false;
        return true;
    }
    return false;
}

/*
 ==========================================================================
 
 Mouse

 ==========================================================================
 */

/*
 =====================================
 Mouse::Mouse
 =====================================
 */
Mouse::Mouse() :
m_pos( 0.0f ) {
	for ( int i = 0; i < 5; ++i ) {
		m_buttonsDown[ i ] = false;
	}
}

/*
 =====================================
 Mouse::SetButtonState
 =====================================
 */
void Mouse::SetButtonState( const MouseButton_t & idx, const bool & isDown ) {
	assert( idx >= 0 && idx < s_maxButtons );

	m_wasButtonsDown[ idx ] = m_buttonsDown[ idx ];
	m_buttonsDown[ idx ] = isDown;
}

/*
 =====================================
 Mouse::WasButtonDown
 =====================================
 */
bool Mouse::WasButtonDown( const MouseButton_t & idx ) {
    if ( m_wasButtonsDown[ idx ] && !m_buttonsDown[ idx ] ) {
        m_wasButtonsDown[ idx ] = false;
        return true;
    }
    return false;
}


#include "Graphics/Graphics.h"
extern int g_screenHeight;

/*
 ================================
 keyboard
 ================================
 */
void keyboard( unsigned char key, int x, int y ) {
    g_keyboard.SetKeyDown( key );
        
//     int mod = glutGetModifiers();
// //     switch ( mod ) {
// //         case GLUT_ACTIVE_CTRL:	{ printf( "Ctrl Held\n" ); } break;
// //         case GLUT_ACTIVE_SHIFT:	{ printf( "Shift Held\n" ); } break;
// //         case GLUT_ACTIVE_ALT:	{ printf( "Alt Held\n" ); } break;
// //     }
// 	if ( mod | GLUT_ACTIVE_SHIFT ) {
// 		g_keyboard.SetKeyDown( Keyboard::kb_SHIFT_IN );
// 	} else {
// 		g_keyboard.SetKeyUp( Keyboard::kb_SHIFT_IN );
// 	}
}

/*
 ================================
 keyboardup
 ================================
 */
void keyboardup( unsigned char key, int x, int y ) {
	g_keyboard.SetKeyUp( key );
}

/*
 ================================
 special
 ================================
 */
bool ignoreRepeats = false;
void special( int key, int x, int y ) {
}

/*
 ================================
 mouse
 ================================
 */
void mouse( int button, int state, int x, int y ) {
/*
// Mouse Buttons
#define  GLUT_LEFT_BUTTON                   0x0000
#define  GLUT_MIDDLE_BUTTON                 0x0001
#define  GLUT_RIGHT_BUTTON                  0x0002

// Button States
#define  GLUT_DOWN                          0x0000
#define  GLUT_UP                            0x0001

// Cursor/Window States
#define  GLUT_LEFT                          0x0000
#define  GLUT_ENTERED                       0x0001
*/
	// Convert from windows coords (origin at top right)
	// To gl coords (origin at lower left)
	y = g_screenHeight - y;

	// Get Left/Right mouse button inputs
	if ( button == GLUT_RIGHT_BUTTON ) {
		if (state == GLUT_DOWN ) {
			g_mouse.SetButtonState( Mouse::mb_right, true );
		} else {
			g_mouse.SetButtonState( Mouse::mb_right, false );
		}
	} else if ( button == GLUT_LEFT_BUTTON ) {
		if ( state == GLUT_DOWN ) {
			g_mouse.SetButtonState( Mouse::mb_left, true );
		} else {
			g_mouse.SetButtonState( Mouse::mb_left, false );
		}
	} else if ( button == GLUT_MIDDLE_BUTTON ) {
		if ( state == GLUT_DOWN ) {
			g_mouse.SetButtonState( Mouse::mb_middle, true );
		} else {
			g_mouse.SetButtonState( Mouse::mb_middle, false );
		}
	}

	// Wheel reports as button 3(scroll up) and button 4(scroll down)
	int mouseZ = (int)g_mouse.ScrollPos();
	if ( 3 == button && GLUT_DOWN == state ) {
		// Mouse wheel scrolled up
		++mouseZ;
	} else if ( 4 == button && GLUT_DOWN == state ) {
		// Mouse wheel scrolled down
		--mouseZ;
	}

	// Update the mouse position
	g_mouse.SetPos( Vec3d( x, y, mouseZ ) );
}

/*
 ================================
 motion

 * Updates mouse position when buttons are pressed
 ================================
 */
void motion( int x, int y ) {
	// Convert from windows coords (origin at top right)
	// To gl coords (origin at lower left)
	y = g_screenHeight - y;

	Vec3d pos( x, y, g_mouse.ScrollPos() );
	g_mouse.SetPos( pos );
}

/*
 ================================
 motionPassive

 * Updates mouse position when zero buttons are pressed
 ================================
 */
void motionPassive( int x, int y ) {
	// Convert from windows coords (origin at top right)
	// To gl coords (origin at lower left)
	y = g_screenHeight - y;

	Vec3d pos( x, y, g_mouse.ScrollPos() );
	g_mouse.SetPos( pos );
}

/*
 ================================
 entry
 ================================
 */
void entry( int state ) {
// 	if ( GLUT_ENTERED == state ) {
// 		printf( "Mouse entered window\n" );
// 	} else {
// 		printf( "Mouse left window area\n" );
// 	}
}