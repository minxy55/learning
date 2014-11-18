// JavaScript Document

var KEY_ENTER = 13;
var KEY_ESC = 27;
var KEY_LEFT = 37;
var KEY_RIGHT = 39;
var KEY_UP = 38;
var KEY_DOWN = 40;
var KEY_0 = 48;
var KEY_1 = 49;
var KEY_2 = 50;
var KEY_3 = 51;
var KEY_4 = 52;
var KEY_5 = 53;
var KEY_6 = 54;
var KEY_7 = 55;
var KEY_8 = 56;
var KEY_9 = 56;
var KEY_a = 65;
var KEY_b = 66;
var KEY_c = 67;
var KEY_d = 68;
var KEY_e = 69;
var KEY_f = 70;
var KEY_g = 71;
var KEY_h = 72;
var KEY_i = 73;
var KEY_j = 74;
var KEY_k = 75;
var KEY_l = 76;
var KEY_m = 77;
var KEY_n = 78;
var KEY_o = 79;
var KEY_p = 80;
var KEY_q = 81;
var KEY_r = 82;
var KEY_s = 83;
var KEY_t = 84;
var KEY_u = 85;
var KEY_v = 86;
var KEY_w = 87;
var KEY_x = 88;
var KEY_y = 89;
var KEY_z = 90;


function isDigitKey(key)
{
	return (key >= 48 && key <= 56);
}

function isArrawKey(key)
{
	return (key >= 37 && key <= 40);
}

function isCharKey(key)
{
	return (key >= 65 && key <= 90);
}

function isEscKey(key)
{
	return (key == KEY_ESC);
}