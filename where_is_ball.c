#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

struct parameter
{
	int win_size_x;
	int win_size_y;
	int ball_rad;
	int dt;
	
};
/*シーケンス条件分岐の列挙型*/
enum sequence
{
	shaffle,
	choose,
	restart,
	stop,
};

/*色の定義*/
const int white = 0;
const int red = 1;
const int green = 2;
const int blue = 3;
const int gray = 9;

/*回転行列*/
void rotate_matrix(int *pos, float rad)
{
	float x, y;
	/*負の方向の丸め誤差をなくすため正方向にバイアス*/
	x = cos(rad) * pos[0] - sin(rad) * pos[1] + 1000;
	y = sin(rad) * pos[0] + cos(rad) * pos[1] + 1000;
	pos[0] = (int)(x + 0.5) - 1000;
	pos[1] = (int)(y + 0.5) - 1000;
}

/*タップした場所を判定する関数*/
int judge_pos (int x, int y, int ball[][3])
{
	int i =0;
	int d = 100000;
	/*3回判定するか、近いものが見つかったら終了。*/
	while (d > 90000)
	{
		if (i < 3)
		{
			d = pow((x - ball[i][0]), 2) + pow((y - ball[i][1]), 2);
		}
		else
		{
			d = i;
		}
		i++;
	}
	return i - 1;
}

/*描画する色を変更する関数*/
void color_func(SDL_Renderer *renderer, int color)
{
	int r = 0x00, g = 0x00, b = 0x00;
	if (color == white)
	{
		r = 0xff;
		g = r;
		b = r;
	}
	else if (color == red)
	{
		r = 0xff;
	}
	else if (color == green)
	{
		g = 0xff;
	}
	else if (color == blue)
	{
		b = 0xff;
	}
	else if (color == gray)
	{
		r = 0x88;
		g = r;
		b = r;
	}
	SDL_SetRenderDrawColor(renderer, r, g, b, SDL_ALPHA_OPAQUE);
}

/*ボールを描画する関数*/
void drow_ball(SDL_Renderer *renderer, int pos_x, int pos_y, int r, int color)
{
	int x, y;
	
	color_func(renderer, color);
	for (x = -r; x <= r; x++)
	{
		y = sqrt(r * r - x * x);
		SDL_RenderDrawLine(renderer, pos_x + x, pos_y - y, pos_x + x, pos_y + y);
	}
}

/*ボールを隠す■を描画する関数*/
void drow_mask(SDL_Renderer *renderer, int pos_x, int pos_y, int l, int color)
{
	/*描画する長方形の領域を定義*/
	SDL_Rect sqare;
	sqare.x = pos_x - (int)(l / 2);
	sqare.y = pos_y - (int)(l / 2);
	sqare.w = l;
	sqare.h = l;
	/*色を設定*/
	color_func(renderer, color);
	/*■をバッファに描画*/
	SDL_RenderFillRect(renderer, &sqare);
}

/*ボールを設置して■で隠す関数*/
void init_func(SDL_Renderer *renderer, int ball[][3], int *d, int num)
{
	int i;
	/*ボールを描画する。numはボールの数*/
	for (i =0; i < 3; i++)
	{
		drow_ball(renderer, ball[i][0], ball[i][1], 100, ball[i][2]);
//	}
	/*■を描画する。■は常に3個*/
//	for (i =0; i < 3; i++)
//	{
		drow_mask(renderer, ball[i][0] + d[i], ball[i][1], 300, gray);
	}
}
/*資格で隠してシャッフルする関数*/
void shaffle_func(SDL_Renderer *renderer, int ball[][3], int n)
{
	int temp[2];
	int center[2];
	int i;
	int flag;
	/*flagは時計回りか反時計回りかを決める変数*/
	flag = (int)(n / 3) * 2 - 1;
	/*回転中心位置決め*/
	center[0] = (ball[n%3][0] + ball[(n + 1) % 3][0])/2;
	center[1] = (ball[n%3][1] + ball[(n + 1) % 3][1])/2;
	/*回転*/
	for (i =0; i < 2; i++)
	{
		temp[0] = ball[(n + i) % 3][0] - center[0];
		temp[1] = ball[(n + i) % 3][1] - center[1];
		rotate_matrix(temp, flag * M_PI/100);
		ball[(n + i) % 3][0] = temp[0] + center[0];
		ball[(n + i) % 3 ][1] = temp[1] + center[1];
	}
	/*■の描画*/
	for (i =0; i < 3; i++)
	{
		drow_mask(renderer, ball[i][0], ball[i][1], 300, gray);
	}
}	

int main(int argc, char *argv[])
{
	int quit_flg = 1;
	enum sequence MODE_FLG;
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Event event;
	
	/* 初期化 */
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
    	fprintf(stderr, "SDL_Init(): %s\n", SDL_GetError());
		return 1;
	}
	/* ウィンドウ作成 */
	window = SDL_CreateWindow("Where is ball?", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1080, 1920, SDL_WINDOW_SHOWN);
	if( window == NULL ) {
		printf("Can not create window\n");
		return 1;
    }
    /*レンダラー呼び出し*/
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (renderer == NULL)
	{
		fprintf(stderr, "SDL_CreateRenderer Error: %s\n", SDL_GetError());
		return 1;
	}
	/*初期状態の定義*/
	MODE_FLG = restart;
	int ball[3][3] = {{300, 300, red},
							{300, 900, white},
							{300, 1500, white}};
	int d[3] = {500, 500, 500};
	int count = 500;
	int n = 0;
	int touchx = -1;
	int touchy = -1;
	int test = 4;
	int refuse = 1;
	
	/* イベントループ */
	while(quit_flg)
	{
		while( SDL_PollEvent(&event) )
		{
			switch (event.type)
			{
				case SDL_MOUSEBUTTONDOWN:
				if (refuse == 0)
				{
					touchx = event.button.x;
					touchy = event.button.y;
				}
//				quit_flg = 0;
//				break;
			}
        }
	
	/*レンダラーで背景描画*/
	SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(renderer);
			
	/*ボールを設置して■で隠すまで*/
	if (MODE_FLG == restart)
	{	
		init_func(renderer, ball, d, 1);
		d[0] -= 10;
		d[1] -= 10;
		d[2] -= 10;
	
		SDL_RenderPresent(renderer);
		/*ボールを隠し終わったら次のシーケンスへ*/
		if (d[1] <= 0)
		{
			MODE_FLG = shaffle;
		}
		SDL_Delay(10);
	}
	/*シャッフルするシーケンス*/
	else if (MODE_FLG == shaffle)
	{
//		int num;
		int amari;
//		num = count / 100;
		amari = count % 100;
		/*シャッフルが終わったら次のシーケンスへ*/
		if (count == 0)
		{
			MODE_FLG = choose;
			refuse = 0;
		}
		else
		{
			/*次のシャッフル決定*/
			if (amari == 0)
			{
				n = rand() % 6;
				SDL_Delay(100);
			}
		}
	/*■の描画*/
		shaffle_func(renderer, ball, n);
		SDL_RenderPresent(renderer);
		SDL_Delay(10);
		count -= 1;	
	}
	
	else if (MODE_FLG == choose)
	{
		test = judge_pos(touchx, touchy, ball);
		if (test < 3)
		{
			refuse = 1;
			init_func(renderer, ball, d, test);
			if(d[test] <500)
			{
				d[test] += 10;
			}

	
			SDL_RenderPresent(renderer);
			if (d[test] >= 500)
			{
//				MODE_FLG = restart;
				SDL_Delay(500);
//				quit_flg = 0;
				refuse = 0;
				test = 4;
			}
		}
		SDL_Delay(10);
		
	}
//	num = -1;
	}
	if (renderer) SDL_DestroyRenderer(renderer);
	if (window) SDL_DestroyWindow(window);

	SDL_Quit();

	return 0;
}