	/** Select button. */
#define	PSP_CTRL_SELECT      0x000001
	/** Start button. */
#define	PSP_CTRL_START       0x000008
	/** Up D-Pad button. */
#define	PSP_CTRL_UP          0x000010
	/** Right D-Pad button. */
#define	PSP_CTRL_RIGHT       0x000020
	/** Down D-Pad button. */
#define	PSP_CTRL_DOWN      	 0x000040
	/** Left D-Pad button. */
#define	PSP_CTRL_LEFT      	 0x000080
	/** Left trigger. */
#define	PSP_CTRL_LTRIGGER    0x000100
	/** Right trigger. */
#define	PSP_CTRL_RTRIGGER    0x000200
	/** Triangle button. */
#define	PSP_CTRL_TRIANGLE    0x001000
	/** Circle button. */
#define	PSP_CTRL_CIRCLE      0x002000
	/** Cross button. */
#define	PSP_CTRL_CROSS       0x004000
	/** Square button. */
#define	PSP_CTRL_SQUARE      0x008000
	/** Home button. */
#define	PSP_CTRL_HOME        0x010000
	/** Hold button. */
#define	PSP_CTRL_HOLD        0x020000
	/** Music Note button. */
#define	PSP_CTRL_NOTE        0x800000

#define MAX_JOYSTICKS		2
//#define TOTAL_PSP_BUTTONS	14
#define TOTAL_PSP_BUTTONS	10
#define DIGITAL_TOL		10000
#define ANALOG_DEADZONE		6000

enum pspButton {
	CROSS = 0,
	CIRCLE,
	SQUARE,
	TRIANGLE,
	RTRIGGER,
	LTRIGGER,
	START,
	SELECT,
	HOME,
	EXIT,
/*
	RIGHT,
	LEFT,
	DOWN,
	UP,
*/
};

const char *map_names[] = {
	"CROSS",
	"CIRCLE",
	"SQUARE",
	"TRIANGLE",
	"RTRIGGER",
	"LTRIGGER",
	"START",
	"SELECT",
	"HOME",
	"EXIT",
};

int jbut[MAX_JOYSTICKS][TOTAL_PSP_BUTTONS];
int jxdigital[MAX_JOYSTICKS];
int jydigital[MAX_JOYSTICKS];
int jxanalog[MAX_JOYSTICKS];
int jyanalog[MAX_JOYSTICKS];
int jtol[MAX_JOYSTICKS];
int jdeadzone[MAX_JOYSTICKS];

char *joymap1 = NULL;
char *joymap2 = NULL;
