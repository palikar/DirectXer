#pragma once

#include <cstdint>
#include <Glm.hpp>
#include <Types.hpp>
#include <Logging.hpp>

enum class KeyCode : uint16_t
{
    // From glfw3.h
    Space      = 32,
    // Apostrophe = 39, /* ' */
    Comma      = 44, /* , */
    Minus      = 45, /* - */
    Period     = 46, /* . */
    Slash      = 47, /* / */

    // LeftControl         = 341,
    // LeftAlt             = 342,
    // LeftShift           = 340,

    // RightControl        = 345,
    // RightAlt            = 346,
    // RightShift          = 344,

    D0 = 48, /* 0 */
    D1 = 49, /* 1 */
    D2 = 50, /* 2 */
    D3 = 51, /* 3 */
    D4 = 52, /* 4 */
    D5 = 53, /* 5 */
    D6 = 54, /* 6 */
    D7 = 55, /* 7 */
    D8 = 56, /* 8 */
    D9 = 57, /* 9 */

    Semicolon = 59, /* ; */
    Equal     = 61, /* = */

    A = 65,
    B = 66,
    C = 67,
    D = 68,
    E = 69,
    F = 70,
    G = 71,
    H = 72,
    I = 73,
    J = 74,
    K = 75,
    L = 76,
    M = 77,
    N = 78,
    O = 79,
    P = 80,
    Q = 81,
    R = 82,
    S = 83,
    T = 84,
    U = 85,
    V = 86,
    W = 87,
    X = 88,
    Y = 89,
    Z = 90,

    LeftBracket  = 91, /* [ */
    Backslash    = 92, /* \ */
    RightBracket = 93, /* ] */
    GraveAccent  = 96, /* ` */

    World1 = 161, /* non-US #1 */
    World2 = 162, /* non-US #2 */

    /* Function keys */
    Escape      = 256,
    Enter       = 257,
    Tab         = 258,
    Backspace   = 259,
    Insert      = 260,
    Delete      = 261,
    Left        = 37,
    Right       = 39,
    Down        = 264,
    Up          = 265,
    PageUp      = 266,
    PageDown    = 267,
    Home        = 268,
    End         = 269,
    CapsLock    = 280,
    ScrollLock  = 281,
    NumLock     = 282,
    PrintScreen = 283,
    Pause       = 284,
    F1          = 290,
    F2          = 291,
    F3          = 292,
    F4          = 293,
    F5          = 294,
    F6          = 295,
    F7          = 296,
    F8          = 297,
    F9          = 298,
    F10         = 121,
    F11         = 122,
    F12         = 301,
    F13         = 302,
    F14         = 303,
    F15         = 304,
    F16         = 305,
    F17         = 306,
    F18         = 307,
    F19         = 308,
    F20         = 309,
    F21         = 310,
    F22         = 311,
    F23         = 312,
    F24         = 313,
    F25         = 314,

    /* Keypad */
    KP0        = 320,
    KP1        = 321,
    KP2        = 322,
    KP3        = 323,
    KP4        = 324,
    KP5        = 325,
    KP6        = 326,
    KP7        = 327,
    KP8        = 328,
    KP9        = 329,
    KPDecimal  = 330,
    KPDivide   = 331,
    KPMultiply = 332,
    KPSubtract = 333,
    KPAdd      = 334,
    KPEnter    = 335,
    KPEqual    = 336,

    LeftShift    = 340,
    LeftControl  = 341,
    LeftAlt      = 342,
    LeftSuper    = 343,
    RightShift   = 344,
    RightControl = 345,
    RightAlt     = 346,
    RightSuper   = 347,
    Menu         = 348
};

enum class MouseCode : uint16_t
{
    BUTTON_1 = 0,
    BUTTON_2 = 1,
    BUTTON_3 = 2,
    BUTTON_4 = 3,
    BUTTON_5 = 4,
    BUTTON_6 = 5,
    BUTTON_7 = 6,
    BUTTON_8 = 7

};


#define GAMEPAD_DPAD_UP 	   0x0001
#define GAMEPAD_DPAD_DOWN 	   0x0002
#define GAMEPAD_DPAD_LEFT 	   0x0004
#define GAMEPAD_DPAD_RIGHT 	   0x0008
#define GAMEPAD_START          0x0010
#define GAMEPAD_BACK 	       0x0020

#define GAMEPAD_LEFT_THUMB 	   0x0040
#define GAMEPAD_RIGHT_THUMB    0x0080

#define GAMEPAD_LEFT_SHOULDER  0x0100
#define GAMEPAD_RIGHT_SHOULDER 0x0200

#define GAMEPAD_A 	           0x1000
#define GAMEPAD_B 	           0x2000
#define GAMEPAD_X 	           0x4000
#define GAMEPAD_Y              0x8000

struct JoystickInput
{
	uint32 Buttons;
	uint16 RightThumbX;
	uint16 RightThumbY;
	
	int16 LeftThumbX;
	int16 LeftThumbY;
	uint8 LeftTrigger;
	uint8 RightTrigger;
};

struct Input
{
	static Input gInput;
	static constexpr int MAX_KEYS = 360;


	// @Imporve use lots of 8 bit things and then bitshift
	bool KeyboardPressedKeys[MAX_KEYS]{0};
	bool KeyboardReleasedKeys[MAX_KEYS]{0};
 
	// @Imporve put in a sinlge 8 bit thing
	bool KeyboardCtrl{ false };
	bool KeyboardShift{ false };
	bool KeyboardAlt{ false };

	glm::vec2 MousePosition{0.0};
	glm::vec2 LastMousePosition{0.0};

	// @Imporve use lots of 8 bit things and then bitshift
	bool MousePressedKeys[10]{0};
	bool MouseReleasedKeys[10]{0};

	glm::vec2 MouseScollOffset{0.0};
	glm::vec2 MouseLastScollOffset{0.0};

	JoystickInput PrevJoystickState{0};
	JoystickInput CurrentJoystickState{0};

	void InitInstance()
	{
	}

	void Reset()
	{
		for (uint32 i = 0; i < MAX_KEYS; ++i)
		{
			KeyboardReleasedKeys[i] = false;
			KeyboardPressedKeys[i] = false;
		}

		for (uint32 i = 0; i < 10; ++i)
		{
			MouseReleasedKeys[i] = false;
			MousePressedKeys[i] = false;
		}
		
		LastMousePosition = MousePosition;
	}

	void Update()
	{
		for (uint32 i = 0; i < MAX_KEYS; ++i)
		{
			KeyboardReleasedKeys[i] = false;
		}

		for (uint32 i = 0; i < 10; ++i)
		{
			MouseReleasedKeys[i] = false;
		}
		
		LastMousePosition = MousePosition;
		
		PrevJoystickState = CurrentJoystickState;
		CurrentJoystickState = {0};
	}

	void UpdateJoystick(JoystickInput state)
	{
		CurrentJoystickState = state;
	}

	void UpdateKeyboardButtonPressed(uint16_t t_Key)
	{
		// @Note Cache miss here!
		KeyboardPressedKeys[t_Key] = true;
		KeyboardReleasedKeys[t_Key] = false;
	}

	void UpdateKeyboardButtonReleased(uint16_t t_Key)
	{
		// @Note Cache miss here!
		KeyboardPressedKeys[t_Key] = false;
		KeyboardReleasedKeys[t_Key] = true;
	}

	void UpdateMousePos(glm::vec2 t_Pos)
	{
		LastMousePosition = MousePosition;
		MousePosition = t_Pos;
	}

	void UpdateMouseButtonPressed(uint32 t_Bnt)
	{
		Assert(t_Bnt < 10, "No such mout botton. There are only 10 mouse buttons.");
		
		MousePressedKeys[t_Bnt] = true;
		MouseReleasedKeys[t_Bnt] = false;
	}
	
	void UpdateMouseButtonReleased(uint32 t_Bnt)
	{
		Assert(t_Bnt < 10, "No such mout botton. There are only 10 mouse buttons.");
		MousePressedKeys[t_Bnt] = false;
		MouseReleasedKeys[t_Bnt] = true;
	}

	void UpdateMouseScroll(float m_Delta)
	{
		MouseScollOffset = {m_Delta, 0};
		MouseLastScollOffset = MouseScollOffset;
	}

	bool IsKeyPressed(KeyCode t_Key)
	{
		return KeyboardPressedKeys[(uint16_t)t_Key];
	}

	bool IsKeyPressed(MouseCode t_Key)
	{
		return MousePressedKeys[(uint16_t)t_Key];
	}

	bool IsKeyReleased(KeyCode t_Key)
	{
		return KeyboardReleasedKeys[(uint16_t)t_Key];
	}

	bool IsKeyReleased(MouseCode t_Key)
	{
		return MouseReleasedKeys[(uint16_t)t_Key];
	}

	bool IsJoystickButtonPressed(uint16 t_Btn)
	{
		return CurrentJoystickState.Buttons & t_Btn;
	}

	bool IsJoystickButtonReleased(uint16 t_Btn)
	{
		return (PrevJoystickState.Buttons & t_Btn) && !(CurrentJoystickState.Buttons & t_Btn);
	}
	
	glm::vec2 GetRightThumb()
	{
		return {(CurrentJoystickState.RightThumbX / (255.0f * 255.0f * 0.5f)), (CurrentJoystickState.RightThumbY / (255.0f * 255.0f * 0.5f))};
	}

	glm::vec2 GetLeftThumb()
	{
		return {(CurrentJoystickState.LeftThumbX / (255.0f * 255.0f * 0.5f)), (CurrentJoystickState.LeftThumbY / (255.0f * 255.0f * 0.5f))};
	}
	
	float GetRightTrigger()
	{
		return CurrentJoystickState.LeftTrigger / (255.0f * 255.0f * 0.5f);
	}
	
	float GetLeftTrigger()
	{
		return CurrentJoystickState.RightTrigger / (255.0f * 255.0f * 0.5f);
	}

    static void Init();


};
