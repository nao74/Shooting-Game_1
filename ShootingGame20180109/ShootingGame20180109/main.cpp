#include "DxLib.h"
#include<math.h>
#include<stdlib.h>
#include<time.h>

#define PLAYERMAX_SHOT 2 //プレイヤーが打てるショット数
#define PLAYER_HP 5 //プレイヤーの残り残機
#define ENEMY_HP 3 //中央の敵のHP

struct SIZE2D {
	int width;
	int height;
};

struct POINT2DF { // 座標を表す
	float x;
	float y;
};
struct VECTOR2DF { // 移動量を表す
	float x;
	float y;
};

struct CHARADATA {
	struct VECTOR2DF move;
	struct POINT2DF pos;
	int r;
	int color;
	BOOL life; //1なら生存、0なら消滅

};

//  ゲーム状態
enum {
	GAME_TITLE,    //タイトル
	GAME_MAIN,     //メイン
	GAME_ENDING,   //エンディング
	GAME_BADEND,   //ゲームオーバー
	GAME_END,      //終了
};

int i = 0, check = 0, Ehit[18], Phit[25], media[7], TextColer, sound[7];
int start, now, currentTime, end, endTime, startcount;
int deg = 0, dDeg = 5, r = 100;
int scrollCount, bgImg, destY[2], srcY[2], srcL[2], imgW, imgH;
int function_status = GAME_TITLE;
char keyBuf[256], message[100];
float rad;
const struct SIZE2D screenSize = { 640, 480 };
const float dt = (float)1 / 60;
const float rad2deg = (float)3.1415926 / 180;
struct VECTOR2DF vec, moveVec;
struct POINT2DF pos[2];
struct CHARADATA shot[PLAYERMAX_SHOT][PLAYERMAX_SHOT]; //プレイヤーshot
struct CHARADATA shot1; //右の敵shot
struct CHARADATA shot2; //左の敵shot
struct CHARADATA shot3; //中央の敵shot
struct CHARADATA shot4; //中央の敵shot2
struct CHARADATA player;
struct CHARADATA enemy[3];
struct CHARADATA object; //中央の公転してるやつ

BOOL EhitFlag; //敵に弾が当たったか
BOOL PhitFlag; //プレイヤーに弾が当たったか
BOOL Ehitcount = ENEMY_HP; //敵のHP
BOOL Phitcount = PLAYER_HP; //プレイヤーのHP
BOOL enemy0shotCount = 0; //敵のショットタイミング
BOOL enemy1shotCount = 0; //敵のショットタイミング
BOOL enemy2shotCount = 0; //敵のショットタイミング

int Title() 
{
	startcount = GetNowCount();

	StopSoundMem(sound[1]);
	StopSoundMem(sound[2]);
	StopSoundMem(sound[3]);
	if (keyBuf[KEY_INPUT_SPACE] == 1) //スペースでゲームスタート
		return GAME_MAIN;
	if (keyBuf[KEY_INPUT_ESCAPE] == 1) //エスケープでゲーム終了
		return GAME_END;

    //文字、画像描写
	DrawGraph(0, 0, bgImg, FALSE);
	SetFontSize(50);
	DrawString(screenSize.width / 8, screenSize.height / 8, "赤、青、緑の弾を避け\n敵宇宙船を破壊せよ！", TextColer);
	SetFontSize(30);
	DrawString(screenSize.width / 4, screenSize.height / 2 - 30, "スペースキー：スタート\n\n", TextColer);
	SetFontSize(20);
	DrawString(100, 300, "エスケープ：終了\n\n", TextColer);
	DrawString(100, 350, "Zキー：弾を打つ\n\n← →：移動", TextColer);

    //リセット
	Ehitcount = ENEMY_HP;
	Phitcount = PLAYER_HP;
	enemy0shotCount = 0;
	enemy2shotCount = 0;
	player.life = 1;
	enemy[0].life = 1;
	enemy[1].life = 1;
	enemy[2].life = 1;

	return GAME_TITLE;
}

int GameMain() 
{

	now = GetNowCount();

	StopSoundMem(sound[0]);

	if (keyBuf[KEY_INPUT_ESCAPE] == 1) //エスケープでゲーム終了
		return GAME_END;

#pragma region デバック用
	
	//if (keyBuf[KEY_INPUT_X] == 1)
	//{
		//Ehitcount = ENEMY_HP;
		//Phitcount = PLAYER_HP;
		//enemy0shotCount = 0;
		//enemy2shotCount = 0;
		//player.life = 1;
		//enemy[0].life = 1;
		//enemy[1].life = 1;
		//enemy[2].life = 1;
	//}

	//if (EhitFlag || PhitFlag)
		//DrawFormatString(screenSize.width / 2, screenSize.height / 2 - 30, GetColor(255, 0, 0), "当たり");
	//else
		//DrawFormatString(screenSize.width / 2, screenSize.height / 2, TextColer, "当たってない");
#pragma endregion

#pragma region 描写

	destY[0] = 0;
	srcL[0] = scrollCount % imgH;
	srcY[0] = imgH - srcL[0];
	if (srcL[0] > screenSize.height)
		srcL[0] = screenSize.height;
	destY[1] = srcL[0];
	srcL[1] = screenSize.height - srcL[0];
	srcY[1] = 0;
	DrawRectGraph(0, destY[0], 0, srcY[0], screenSize.width, srcL[0], bgImg, FALSE, FALSE);
	DrawRectGraph(0, destY[1], 0, srcY[1], screenSize.width, srcL[1], bgImg, FALSE, FALSE);
	scrollCount++;

	DrawFormatString(screenSize.width / 2, screenSize.height / 2 + 60, TextColer, "プレイヤー残り残機：%d機", Phitcount);
	currentTime = (now - startcount) / 1000; //経過時間
	DrawFormatString(screenSize.width / 2, screenSize.height / 2 + 30, TextColer, "タイム：%d秒", currentTime);
	DrawFormatString(screenSize.width / 8, 30, TextColer, "敵ボス残機：%d機", Ehitcount);

#pragma endregion

	//プレイヤー
	if (player.life)
	{
		DrawCircle(player.pos.x, player.pos.y, player.r, GetColor(255, 255, 255), TRUE); //プレイヤーの弾
		DrawGraph(player.pos.x - 50, player.pos.y - 50, media[0], TRUE); //プレイヤーの画像
	}

	if (player.life) //プレイヤーが生きていたら
	{
		if (keyBuf[KEY_INPUT_LEFT] == 1)
		{
			if (player.pos.x > 30)
			{
				player.pos.x -= 5;
			}
		}
		if (keyBuf[KEY_INPUT_RIGHT] == 1)
		{
			if (player.pos.x + 30 < 640)
			{
				player.pos.x += 5;
			}
		}

		//ショット
		if (i >= PLAYERMAX_SHOT)
			i = 0;
		if (keyBuf[KEY_INPUT_Z] == 1)
		{
			if (check == 0)
			{
				for (i = 0; i < PLAYERMAX_SHOT; i++)
				{
					if (!shot[i][i].life)
					{
						shot[i][i].life = 1;
						shot[i][i].pos.x = player.pos.x;
						shot[i][i].pos.y = player.pos.y;
						shot[i][i].move.y = -10;
						shot[i][i].r = 5;
						shot[i][i].color = GetColor(255, 255, 255);
						break;
					}
				}
				check = 1;
				PlaySoundMem(sound[4], DX_PLAYTYPE_BACK);
			}
		}
		else
			check = 0;
	}
	EhitFlag = 0;
	PhitFlag = 0;

	

#pragma region 敵とのショット当たり判定

	for (i = 0; i < PLAYERMAX_SHOT; i++)
	{
		if (shot[i][i].life)
		{
			shot[i][i].pos.y += shot[i][i].move.y;

			if (0 > shot[i][i].pos.y + shot[i][i].r)//プレイヤーの弾が0を超えたら
				shot[i][i].life = 0;

			else//プレイヤーの弾描写
			DrawCircle(shot[i][i].pos.x - 7, shot[i][i].pos.y, shot[i][i].r, shot[i][i].color, TRUE);
			DrawCircle(shot[i][i].pos.x + 7, shot[i][i].pos.y, shot[i][i].r, shot[i][i].color, TRUE);

			if (enemy[0].life) //左の敵の当たり判定(赤）
			{
				Phit[0] = enemy[0].pos.x - shot[i][i].pos.x - 7; Phit[1] = enemy[0].pos.y - shot[i][i].pos.y; Phit[2] = enemy[0].r + shot[i][i].r;
				Phit[3] = enemy[0].pos.x - shot[i][i].pos.x + 7; Phit[4] = enemy[0].pos.y - shot[i][i].pos.y; Phit[5] = enemy[0].r + shot[i][i].r;
				if (Phit[0] * Phit[0] + Phit[1] * Phit[1] < Phit[2] * Phit[2] ||
					Phit[3] * Phit[3] + Phit[4] * Phit[4] < Phit[5] * Phit[5])
				{
					PhitFlag = 1;
					enemy[0].life = 0;
					shot[i][i].life = 0;
					shot1.life = 0;
					DrawGraph(enemy[0].pos.x / 6, enemy[0].pos.y / 3, media[6], TRUE);
					PlaySoundMem(sound[6], DX_PLAYTYPE_BACK);
				}
			}

			if (enemy[1].life) //右の敵の当たり判定(青)
			{
				Phit[6] = enemy[1].pos.x - shot[i][i].pos.x - 7; Phit[7] = enemy[1].pos.y - shot[i][i].pos.y; Phit[8] = enemy[1].r + shot[i][i].r;
				Phit[9] = enemy[1].pos.x - shot[i][i].pos.x + 7; Phit[10] = enemy[1].pos.y - shot[i][i].pos.y; Phit[11] = enemy[1].r + shot[i][i].r;
				if (Phit[6] * Phit[6] + Phit[7] * Phit[7] < Phit[8] * Phit[8] ||
					Phit[9] * Phit[9] + Phit[10] * Phit[10] < Phit[11] * Phit[11])
				{
					PhitFlag = 1;
					enemy[1].life = 0;
					shot[i][i].life = 0;
					shot2.life = 0;
					DrawGraph(enemy[1].pos.x - 70, enemy[1].pos.y - 70, media[6], TRUE);
					PlaySoundMem(sound[6], DX_PLAYTYPE_BACK);
				}
			}

			if (enemy[2].life) //中央の敵の当たり判定(緑)
			{
				Phit[12] = enemy[2].pos.x - shot[i][i].pos.x - 7; Phit[13] = enemy[2].pos.y - shot[i][i].pos.y; Phit[14] = enemy[2].r + shot[i][i].r;
				Phit[15] = enemy[2].pos.x - shot[i][i].pos.x + 7; Phit[16] = enemy[2].pos.y - shot[i][i].pos.y; Phit[17] = enemy[2].r + shot[i][i].r;
				if (Phit[12] * Phit[12] + Phit[13] * Phit[13] < Phit[14] * Phit[14] ||
					Phit[15] * Phit[15] + Phit[16] * Phit[16] < Phit[17] * Phit[17])
				{
					PhitFlag = 1;
					shot[i][i].life = 0;
					Ehitcount--;//当たったらカウント-１

					if (Ehitcount == 0)//カウントが0になったら敵が消える
					{
						enemy[2].life = 0;
						end = GetNowCount();
						PlaySoundMem(sound[6], DX_PLAYTYPE_BACK);
						PlaySoundMem(sound[2], DX_PLAYTYPE_BACK);
						return GAME_ENDING;
					}
				}
			}

			Phit[19] = object.pos.x - shot[i][i].pos.x; Phit[20] = object.pos.y - shot[i][i].pos.y; Phit[21] = object.r + shot[i][i].r;
			Phit[22] = object.pos.x - shot[i][i].pos.x; Phit[23] = object.pos.y - shot[i][i].pos.y; Phit[24] = object.r + shot[i][i].r;
			if (Phit[19] * Phit[19] + Phit[20] * Phit[20] < Phit[21] * Phit[21] ||
				Phit[22] * Phit[22] + Phit[23] * Phit[23] < Phit[24] * Phit[24])
			{
				PhitFlag = 1;
				shot[i][i].life = 0;
			}
		}
	}
#pragma endregion

#pragma region 敵の行動

	if (enemy[0].life)//左の敵が生きていたら	
	{
		DrawCircle(enemy[0].pos.x, enemy[0].pos.y, 20, GetColor(255, 0, 0), TRUE);//敵１
		DrawGraph(enemy[0].pos.x / 6, enemy[0].pos.y / 3, media[1], TRUE);

		if (player.life)
		{
			enemy0shotCount++;//敵のショットタイミング

			if (enemy0shotCount == 3)//3秒に1回打つ
			{
				if (!shot1.life)
				{
					shot1.pos.x = enemy[0].pos.x;
					shot1.pos.y = enemy[0].pos.y;
					shot1.r = 10;
					{
						vec.x = player.pos.x - shot1.pos.x;
						vec.y = player.pos.y - shot1.pos.y;
						shot1.move.x = vec.x;
						shot1.move.y = vec.y;
					}
					shot1.life = 1;
					PlaySoundMem(sound[5], DX_PLAYTYPE_BACK);
				}
				enemy0shotCount = 0;
			}
			if (shot1.life)
			{
				shot1.pos.x += shot1.move.x * dt;
				shot1.pos.y += shot1.move.y * dt;

				if (shot1.pos.y > 480 || shot1.pos.y < 0 ||
					shot1.pos.x > 640 || shot1.pos.x < 0)
					shot1.life = 0;

				//左の敵の弾
				DrawCircle(shot1.pos.x, shot1.pos.y, shot1.r, GetColor(255, 0, 0), TRUE);

				if (player.life)
				{
					Ehit[0] = player.pos.x - shot1.pos.x; Ehit[1] = player.pos.y - shot1.pos.y; Ehit[2] = player.r + shot1.r;
					if (Ehit[0] * Ehit[0] + Ehit[1] * Ehit[1] < Ehit[2] * Ehit[2])
					{
						EhitFlag = 1;//敵の弾があったらflagを立てる
						shot1.life = 0;
					}
				}
			}
		}
	}

	if (enemy[1].life)//右の敵が生きていたら
	{
		DrawCircle(enemy[1].pos.x, enemy[1].pos.y, 20, GetColor(0, 0, 255), TRUE);//敵2
		DrawGraph(enemy[1].pos.x - 70, enemy[1].pos.y - 70, media[2], TRUE);

		if (player.life)
		{
			enemy1shotCount++;

			if (enemy1shotCount == 6)//6秒に1回打つ
			{
				if (!shot2.life)
				{
					shot2.pos.x = enemy[1].pos.x;
					shot2.pos.y = enemy[1].pos.y;
					shot2.r = 10;
					{
						vec.x = player.pos.x - shot2.pos.x;
						vec.y = player.pos.y - shot2.pos.y;
						shot2.move.x = vec.x;
						shot2.move.y = vec.y;

					}
					shot2.life = 1;
					PlaySoundMem(sound[5], DX_PLAYTYPE_BACK);
				}
				enemy1shotCount = 0;
			}

			if (shot2.life)
			{
				shot2.pos.x += shot2.move.x * dt / 2;
				shot2.pos.y += shot2.move.y * dt / 2;

				if (shot2.pos.y > 480 || shot2.pos.y < 0 ||
					shot2.pos.x > 640 || shot2.pos.x < 0)
					shot2.life = 0;

				//右の敵の弾
				DrawCircle(shot2.pos.x, shot2.pos.y, shot2.r, GetColor(0, 0, 255), TRUE);

				if (player.life)
				{
					Ehit[3] = player.pos.x - shot2.pos.x; Ehit[4] = player.pos.y - shot2.pos.y; Ehit[5] = player.r + shot2.r;
					if (Ehit[3] * Ehit[3] + Ehit[4] * Ehit[4] < Ehit[5] * Ehit[5])
					{
						EhitFlag = 1;
						shot2.life = 0;
					}
				}
			}
		}
	}

	if (enemy[2].life)//中央の敵が生きていたら
	{
		DrawCircle(enemy[2].pos.x, enemy[2].pos.y, enemy[2].r, GetColor(0, 255, 0), TRUE);//敵3
		DrawGraph(enemy[2].pos.x - 75, enemy[2].pos.y - 80, media[3], TRUE);

		if (player.life)
		{
			enemy2shotCount++;

			if (enemy2shotCount == 60 || !shot4.life)
			{
				if (!shot3.life)
				{
					shot3.pos.x = enemy[2].pos.x;
					shot3.pos.y = enemy[2].pos.y;
					shot3.r = 30;
					{
						shot3.move.y = 6;
					}
					shot3.life = 1;
					PlaySoundMem(sound[5], DX_PLAYTYPE_BACK);
				}
				enemy2shotCount = 0;
			}

			if (shot3.life)
			{
				shot3.pos.y += shot3.move.y;

				if (shot3.pos.y > 480 || shot3.pos.x > 640)
					shot3.life = 0;

				//中央の敵の弾
				DrawCircle(shot3.pos.x + 70, shot3.pos.y, shot3.r, GetColor(0, 255, 0), TRUE);
				DrawCircle(shot3.pos.x - 70, shot3.pos.y, shot3.r, GetColor(0, 255, 0), TRUE);

				if (player.life)
				{
					Ehit[6] = player.pos.x - shot3.pos.x + 70; Ehit[7] = player.pos.y - shot3.pos.y; Ehit[8] = player.r + shot3.r;
					Ehit[9] = player.pos.x - shot3.pos.x - 70; Ehit[10] = player.pos.y - shot3.pos.y; Ehit[11] = player.r + shot3.r;

					if (Ehit[6] * Ehit[6] + Ehit[7] * Ehit[7] < Ehit[8] * Ehit[8] ||
						Ehit[9] * Ehit[9] + Ehit[10] * Ehit[10] < Ehit[11] * Ehit[11])
					{
						EhitFlag = 1;
						shot3.life = 0;
					}
				}
			}

			enemy2shotCount++;

			if (enemy2shotCount == 60 || !shot3.life)
			{
				if (!shot4.life)
				{
					shot4.pos.x = enemy[2].pos.x;
					shot4.pos.y = enemy[2].pos.y;
					shot4.r = 50;
					{
						shot4.move.y = 5;
					}
					shot4.life = 1;
					PlaySoundMem(sound[5], DX_PLAYTYPE_BACK);
				}
				enemy2shotCount = 0;
			}

			if (shot4.life)
			{
				shot4.pos.y += shot4.move.y;

				if (shot4.pos.y > 480 || shot4.pos.x > 640)
					shot4.life = 0;

				//中央の敵の弾2
				DrawCircle(shot4.pos.x, shot4.pos.y, shot4.r, GetColor(0, 255, 0), TRUE);

				if (player.life)
				{
					Ehit[12] = player.pos.x - shot4.pos.x; Ehit[13] = player.pos.y - shot4.pos.y; Ehit[14] = player.r + shot4.r;
					if (Ehit[12] * Ehit[12] + Ehit[13] * Ehit[13] < Ehit[14] * Ehit[14])
					{
						EhitFlag = 1;
						shot4.life = 0;
					}
					for (i = 0; i < PLAYERMAX_SHOT; i++)
					{
						Ehit[15] = shot4.pos.x - shot[i][i].pos.x; Ehit[16] = shot4.pos.y - shot[i][i].pos.y; Ehit[17] = shot4.r - shot[i][i].r;
						if (Ehit[15] * Ehit[15] + Ehit[16] * Ehit[16] < Ehit[17] * Ehit[17])
						{
							shot[i][i].life = 0;
						}
					}
				}
			}
		}
	}
#pragma endregion

	//プレイヤーの生死
	if (EhitFlag)
	{
		Phitcount--; //flagが立つたびにcountをマイナス

		if (Phitcount == 0)
		{
			player.life = 0;
			DrawGraph(player.pos.x - 50, player.pos.y - 50, media[6], TRUE);
			PlaySoundMem(sound[6], DX_PLAYTYPE_BACK);
			PlaySoundMem(sound[3], DX_PLAYTYPE_BACK);
			return GAME_BADEND;
		}
	}

	//中央で回転しているオブジェクト
	rad = (deg % 360) * rad2deg; //radが１フレームごとに変化
	object.pos.x = r * cos(rad) + screenSize.width / 2;
	object.pos.y = r * sin(rad) + enemy[2].pos.y / 2;
	deg += dDeg;
	DrawCircle(object.pos.x, object.pos.y, object.r, object.color, TRUE);

	return GAME_MAIN;
}

int Ending() //クリアしたら
{
	StopSoundMem(sound[0]);
	StopSoundMem(sound[1]);
	shot[PLAYERMAX_SHOT][PLAYERMAX_SHOT].life = 0; //警告が出る
	shot1.life = 0;
	shot2.life = 0;
	shot3.life = 0;
	DrawGraph(0, 0, bgImg, FALSE);
	DrawGraph(screenSize.width / 6, screenSize.height / 6, media[4], TRUE);
	DrawGraph(screenSize.width / 2, screenSize.height / 2 + 30, media[5], TRUE);
	SetFontSize(60);
	DrawFormatString(screenSize.width / 6, screenSize.height / 6, TextColer, "クリア！");
	SetFontSize(20);
	DrawFormatString(screenSize.width / 6, screenSize.height / 4 + 50, TextColer, "クリアタイム：%3d秒", (end - startcount) / 1000);
	DrawFormatString(screenSize.width / 6, screenSize.height / 4 + 80, TextColer, "エスケープキーで終了\nXキーでタイトルへ");
	if (keyBuf[KEY_INPUT_ESCAPE] == 1)
		return GAME_END;
	if (keyBuf[KEY_INPUT_X] == 1)
		return GAME_TITLE;
	return GAME_ENDING;
}
int BadEnd() //クリア出来なかったら
{
	StopSoundMem(sound[0]);
	StopSoundMem(sound[1]);
	shot1.life = 0;
	shot2.life = 0;
	shot3.life = 0;
	DrawGraph(0, 0, bgImg, FALSE);
	DrawGraph(player.pos.x + 100, player.pos.y / 2, media[0], TRUE);
	DrawGraph(player.pos.x + 100, player.pos.y / 2, media[6], TRUE);
	SetFontSize(60);
	DrawFormatString(screenSize.width / 6, screenSize.height / 6, TextColer, "残念！");
	SetFontSize(20);
	DrawFormatString(screenSize.width / 6, screenSize.height / 4 + 80, TextColer, "エスケープキーで終了\nXキーでタイトルへ");
	if (keyBuf[KEY_INPUT_ESCAPE] == 1)
		return GAME_END;
	if (keyBuf[KEY_INPUT_X] == 1)
		return GAME_TITLE;
	return GAME_BADEND;
}

int WINAPI WinMain(HINSTANCE hI, HINSTANCE hP, LPSTR lpC, int nC)
{

	ChangeWindowMode(TRUE);
	if (DxLib_Init() == -1)
		return -1;
	SetDrawScreen(DX_SCREEN_BACK);
	TextColer = GetColor(255, 255, 255);

	//プレイヤー情報
	player.pos.x = (float)(screenSize.width / 2);
	player.pos.y = 430;
	player.r = 30;
	player.life = 1;
	media[0] = LoadGraph(".\\media\\illustrain08_space08.png");
	//右の敵情報
	enemy[0].pos.x = screenSize.width / 8;
	enemy[0].pos.y = 100;
	enemy[0].r = 50;
	enemy[0].life = 1;
	enemy[0].color = GetColor(255, 0, 0);
	media[1] = LoadGraph(".\\media\\illustrain10-utyu03.png");
	//左の敵情報
	enemy[1].pos.x = screenSize.width - 80;
	enemy[1].pos.y = 100;
	enemy[1].r = 50;
	enemy[1].life = 1;
	enemy[1].color = GetColor(0, 0, 255);
	media[2] = LoadGraph(".\\media\\illustrain10-utyu04.png");
	//中央の敵情報
	enemy[2].pos.x = screenSize.width / 2;
	enemy[2].pos.y = 30;
	enemy[2].r = 40;
	enemy[2].life = 1;
	enemy[2].color = GetColor(0, 255, 0);
	media[3] = LoadGraph(".\\media\\illustrain08_space07.png");
	//公転してるやつ
	object.r = 30;
	object.color = GetColor(255, 255, 0);
	//背景
	bgImg = LoadGraph(".\\media\\bg_uchu_space.png");
	GetGraphSize(bgImg, &imgW, &imgH);
	scrollCount = screenSize.height;
	//クリア画面
	media[4] = LoadGraph(".\\media\\illustrain09-utyuu8.png"); //月面
	media[5] = LoadGraph(".\\media\\illustrain08_space09.png"); //パイロット
	//エフェクト
	media[6] = LoadGraph(".\\media\\bakuhatsu.png");
	//BGM
	sound[0] = LoadSoundMem(".\\music\\bgm_maoudamashii_cyber44.mp3"); //タイトル
	PlaySoundMem(sound[0], DX_PLAYTYPE_BACK);
	sound[1] = LoadSoundMem(".\\music\\bgm_maoudamashii_8bit18.mp3"); //戦闘
	sound[2] = LoadSoundMem(".\\music\\bgm_maoudamashii_8bit22.mp3"); //クリア画面
	sound[3] = LoadSoundMem(".\\music\\bgm_maoudamashii_8bit20.mp3"); //ゲームオーバー画面
	//効果音
	sound[4] = LoadSoundMem(".\\music\\beamgun-shot1.mp3"); //プレイヤーショット音
	sound[5] = LoadSoundMem(".\\music\\shot1.mp3"); //敵ショット音
	sound[6] = LoadSoundMem(".\\music\\se_maoudamashii_explosion06.mp3"); //爆発音

	for (i = 0; i < PLAYERMAX_SHOT; i++)
		shot[i][i].life = 0;

	start = GetNowCount();

	while (ProcessMessage() == 0) //画面切り替え
	{
		GetHitKeyStateAll(keyBuf); // すべてのキーの状態を得る
		ClearDrawScreen();

		switch (function_status) {

		case GAME_TITLE:
			function_status = Title();
			PlaySoundMem(sound[1], DX_PLAYTYPE_BACK);
			break;
		case GAME_MAIN:
			function_status = GameMain();
			break;
		case GAME_ENDING:
			function_status = Ending();
			PlaySoundMem(sound[0], DX_PLAYTYPE_BACK);
			break;
		case GAME_BADEND:
			function_status = BadEnd();
			PlaySoundMem(sound[0], DX_PLAYTYPE_BACK);
			break;
		default:
			DxLib_End(); // ＤＸライブラリ使用の終了処理
			return 0;
			break;
		}

		if (ProcessMessage() == -1) break;  //エラーが起きたら終了

		ScreenFlip();
	}
	DxLib_End();
	return 0;
}

