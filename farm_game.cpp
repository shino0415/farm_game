#include <stdio.h>
#include <stdlib.h>
#include <glut.h>
#include <windows.h>
#include <mmsystem.h>

// 配列の箱の最大
const int MAX_SIZE = 9;
// 今使ってる範囲
int gridSize_x = 3;
int gridSize_z = 3;

// 選択しているマス
int select_x = 1;
int select_z = 1;

// 作物の種類数
const int CROP_COUNT = 4;   // CROP_NONE, WHEAT, CARROT, TOMATO

// レシピ1つあたりでできる作物の種類の最大数
const int MAX_RESULTS = 4;
// レシピの最大数
const int MAX_RECIPES = 16;
// レシピの個数
int recipeCount = 0;

// SEED期間
int SeedTime = 3;
// GROWING期間
int GrowingTime = 10;

// 種のインベントリ
int seedCount[CROP_COUNT];
// 作物のインベントリ
int cropCount[CROP_COUNT];

// 土の状態
enum CropStage {
	EMPTY,      // 空
	SEED,       // 種
	GROWING,    // 成長中
	MATURE      // 成熟
};

enum CropType {
	CROP_NONE,    // 0 = なし
	CROP_WHEAT,   // 1 = 小麦
	CROP_CARROT,  // 2 = ニンジン
	CROP_TOMATO   // 3 = トマト
};

// 植物フォーマット
struct Cell {
	CropStage stage;
	int cropType;
	int growthTime;
};

// 交配結果のフォーマット
struct CrossbreedResult {
	int cropType;
	int weight;
};

// 交配レシピのフォーマット
struct CrossbreedRecipe {
	int typeA;
	int typeB;
	CrossbreedResult results[MAX_RESULTS];
	int resultCount;
};

CrossbreedRecipe recipes[MAX_RECIPES];

// レシピ設定
void initRecipes(void) {
	recipes[0].typeA = CROP_WHEAT;
	recipes[0].typeB = CROP_WHEAT;
	recipes[0].results[0].cropType = CROP_WHEAT;
	recipes[0].results[0].weight = 70;
	recipes[0].results[1].cropType = CROP_CARROT;
	recipes[0].results[1].weight = 30;
	recipes[0].resultCount = 2;
	recipeCount += 1;

	recipes[1].typeA = CROP_WHEAT;
	recipes[1].typeB = CROP_CARROT;
	recipes[1].results[0].cropType = CROP_WHEAT;
	recipes[1].results[0].weight = 50;
	recipes[1].results[1].cropType = CROP_CARROT;
	recipes[1].results[1].weight = 50;
	recipes[1].resultCount = 2;
	recipeCount += 1;

	recipes[2].typeA = CROP_CARROT;
	recipes[2].typeB = CROP_CARROT;
	recipes[2].results[0].cropType = CROP_CARROT;
	recipes[2].results[0].weight = 70;
	recipes[2].results[1].cropType = CROP_TOMATO;
	recipes[2].results[1].weight = 30;
	recipes[2].resultCount = 2;
	recipeCount += 1;

	recipes[3].typeA = CROP_WHEAT;
	recipes[3].typeB = CROP_TOMATO;
	recipes[3].results[0].cropType = CROP_WHEAT;
	recipes[3].results[0].weight = 60;
	recipes[3].results[1].cropType = CROP_TOMATO;
	recipes[3].results[1].weight = 40;
	recipes[3].resultCount = 2;
	recipeCount += 1;

	recipes[4].typeA = CROP_CARROT;
	recipes[4].typeB = CROP_TOMATO;
	recipes[4].results[0].cropType = CROP_CARROT;
	recipes[4].results[0].weight = 60;
	recipes[4].results[1].cropType = CROP_TOMATO;
	recipes[4].results[1].weight = 40;
	recipes[4].resultCount = 2;
	recipeCount += 1;

	recipes[5].typeA = CROP_TOMATO;
	recipes[5].typeB = CROP_TOMATO;
	recipes[5].results[0].cropType = CROP_TOMATO;
	recipes[5].results[0].weight = 100;
	recipes[5].resultCount = 1;
	recipeCount += 1;
}

// インベントリの初期化
void initInventory(void) {
	for (int i = 0; i < CROP_COUNT; i++) {
		seedCount[i] = 0;
		cropCount[i] = 0;
	}
	seedCount[CROP_WHEAT] = 2;
}

// 土のマス目
Cell grid[MAX_SIZE][MAX_SIZE];
void initGrid(void) {
	for (int i = 0; i < MAX_SIZE; i++) {
		for (int j = 0; j < MAX_SIZE; j++) {
			grid[i][j].stage = EMPTY;
			grid[i][j].cropType = CROP_NONE;
			grid[i][j].growthTime = 0;
		}
	}
}

int isValid(int x, int z) {
	if (x < 0) {
		return 0;
	}
	if (x >= gridSize_x) {
		return 0;
	}
	if (z < 0) {
		return 0;
	}
	if (z >= gridSize_z) {
		return 0;
	}
	return 1;
}

// (x,z) が範囲内で、かつ MATURE なら 1 を返す
int isMature(int x, int z) {
	if (!isValid(x, z)) {
		return 0;
	}
	if (grid[x][z].stage != MATURE) {
		return 0;
	}
	return 1;
}

// 2つの作物の組み合わせに合うレシピの番号を返す。（なければ-1を返す）
int findRecipe(int typeA, int typeB) {
	for (int r = 0; r < recipeCount; r++) {
		// 増加方向でマッチ
		if (recipes[r].typeA == typeA && recipes[r].typeB == typeB) {
			return r;
		}
		// 減少方向でマッチ
		if (recipes[r].typeA == typeB && recipes[r].typeB == typeA) {
			return r;
		}
	}
	return -1;   // 該当なし
}

void checkCrossbreed(int x, int z)
{
	// 空きマスじゃなければ交配しない
	if (grid[x][z].stage != EMPTY) {
		return;
	}

	// 交配結果の重み
	int weights[CROP_COUNT] = {};

	// 上下左右の座標
	int up_x = x;
	int	up_z = z - 1;
	int down_x = x;
	int down_z = z + 1;
	int left_x = x - 1;
	int left_z = z;
	int right_x = x + 1;
	int right_z = z;

	// 上下左右のマスが MATURE かどうか判定
	int upMature = isMature(up_x, up_z);
	int downMature = isMature(down_x, down_z);
	int leftMature = isMature(left_x, left_z);
	int rightMature = isMature(right_x, right_z);

	// 上×左、左×下、下×右、右×上のどれかがMATUREなら、低確率でSEEDにする
	int recipeIndex = -1;
	if (upMature && leftMature) {
		recipeIndex = findRecipe(grid[up_x][up_z].cropType, grid[left_x][left_z].cropType);
		if (recipeIndex != -1) {
			for (int i = 0; i < recipes[recipeIndex].resultCount; i++) {
				int cropType = recipes[recipeIndex].results[i].cropType;
				int weight = recipes[recipeIndex].results[i].weight;
				weights[cropType] += weight;
			}
		}
	}
	if (leftMature && downMature) {
		recipeIndex = findRecipe(grid[left_x][left_z].cropType, grid[down_x][down_z].cropType);
		if (recipeIndex != -1) {
			for (int i = 0; i < recipes[recipeIndex].resultCount; i++) {
				int cropType = recipes[recipeIndex].results[i].cropType;
				int weight = recipes[recipeIndex].results[i].weight;
				weights[cropType] += weight;
			}
		}
	}
	if (downMature && rightMature) {
		recipeIndex = findRecipe(grid[down_x][down_z].cropType, grid[right_x][right_z].cropType);
		if (recipeIndex != -1) {
			for (int i = 0; i < recipes[recipeIndex].resultCount; i++) {
				int cropType = recipes[recipeIndex].results[i].cropType;
				int weight = recipes[recipeIndex].results[i].weight;
				weights[cropType] += weight;
			}
		}
	}
	if (rightMature && upMature) {
		recipeIndex = findRecipe(grid[right_x][right_z].cropType, grid[up_x][up_z].cropType);
		if (recipeIndex != -1) {
			for (int i = 0; i < recipes[recipeIndex].resultCount; i++) {
				int cropType = recipes[recipeIndex].results[i].cropType;
				int weight = recipes[recipeIndex].results[i].weight;
				weights[cropType] += weight;
			}
		}
	}
	int weightSum = 0;
	for (int i = 0; i < CROP_COUNT; i++) {
		weightSum += weights[i];
	}
	if (weightSum == 0) {
		return;
	}
	if (rand() % 100 < 5) { // 5%の確率で交配
		int randValue = rand() % weightSum;
		int cumulativeWeight = 0;
		for (int i = 0; i < CROP_COUNT; i++) {
			cumulativeWeight += weights[i];
			if (randValue < cumulativeWeight) {
				grid[x][z].stage = SEED;
				grid[x][z].cropType = i;
				grid[x][z].growthTime = 0;
				break;
			}
		}
	}
}

void checkAllCrossbreed(void) {
	for (int i = 0; i < gridSize_x; i++) {
		for (int j = 0; j < gridSize_z; j++) {
			checkCrossbreed(i, j);
		}
	}
}


void timer(int value)
{
	for (int i = 0; i < gridSize_x; i++) {
		for (int j = 0; j < gridSize_z; j++) {
			// 作物の成長
			if (grid[i][j].stage == SEED) {
				if (grid[i][j].growthTime < SeedTime) {
					grid[i][j].growthTime += 1;
				}
				else {
					grid[i][j].stage = GROWING;
					grid[i][j].growthTime = 0;
				}
			}
			else if (grid[i][j].stage == GROWING) {
				if (grid[i][j].growthTime < GrowingTime) {
					grid[i][j].growthTime += 1;
				}
				else {
					grid[i][j].stage = MATURE;
					grid[i][j].growthTime = 0;
				}
			}
		}
	}
	checkAllCrossbreed();
	glutPostRedisplay();
	glutTimerFunc(1000, timer, 0);
}

// 選択中の種
int selectedSeed = CROP_WHEAT;

void keyboard(unsigned char key, int x, int y)
{
	switch (key) {
		case 'p':
			printf("(%3d,%3d)でpが押されました\n", x, y);
			if (grid[select_x][select_z].stage == EMPTY && seedCount[selectedSeed] > 0) {
				seedCount[selectedSeed] -= 1;
				grid[select_x][select_z].stage = SEED;
				grid[select_x][select_z].cropType = selectedSeed;
				grid[select_x][select_z].growthTime = 0;
			}
			break;
		case 'h':
			if (grid[select_x][select_z].stage == MATURE) {
				int type = grid[select_x][select_z].cropType;
				seedCount[type] += 2;
				cropCount[type] += 1;
				grid[select_x][select_z].stage = EMPTY;
				grid[select_x][select_z].cropType = CROP_NONE;
				grid[select_x][select_z].growthTime = 0;
				printf("収穫: type=%d 種=%d 作物=%d\n", type, seedCount[type], cropCount[type]);
			}
			break;
		case '1':
			selectedSeed = CROP_WHEAT;
			printf("(%3d,%3d)で1が押されました\n", x, y);
			break;
		case '2':
			selectedSeed = CROP_CARROT;
			printf("(%3d,%3d)で2が押されました\n", x, y);
			break;
		case '3':
			selectedSeed = CROP_TOMATO;
			printf("(%3d,%3d)で3が押されました\n", x, y);
			break;
	}
}

void keyboardUp(unsigned char key, int x, int y)
{
	switch (key) {
	case '1':
		printf("(%3d,%3d)で1が離されました\n", x, y);
		break;
	case '2':
		printf("(%3d,%3d)で2が離されました\n", x, y);
		break;
	case '3':
		printf("(%3d,%3d)で3が離されました\n", x, y);
		break;
	}
}

void specialKey(int key, int x, int y)
{
	switch (key) {
		case GLUT_KEY_UP:
			printf("(%3d,%3d)で[↑]が押されました\n", x, y);
			if (select_z > 0) {
				select_z -= 1;
			}
			break;
		case GLUT_KEY_DOWN:
			printf("(%3d,%3d)で[↓]が押されました\n", x, y);
			if (select_z < gridSize_z - 1) {
				select_z += 1;
			}
			break;
		case GLUT_KEY_LEFT:
			printf("(%3d,%3d)で[←]が押されました\n", x, y);
			if (select_x > 0) {
				select_x -= 1;
			}
			break;
		case GLUT_KEY_RIGHT:
			printf("(%3d,%3d)で[→]が押されました\n", x, y);
			if (select_x < gridSize_x - 1) {
				select_x += 1;
			}
			break;
	}
	glutPostRedisplay();
}

void specialKeyUp(int key, int x, int y)
{
	switch (key) {
		case GLUT_KEY_UP:
			printf("(%3d,%3d)で[↑]が離されました\n", x, y);
			break;
		case GLUT_KEY_DOWN:
			printf("(%3d,%3d)で[↓]が離されました\n", x, y);
			break;
		case GLUT_KEY_LEFT:
			printf("(%3d,%3d)で[←]が離されました\n", x, y);
			break;
		case GLUT_KEY_RIGHT:
			printf("(%3d,%3d)で[→]が離されました\n", x, y);
			break;
	}
}

static int MouseLB_ON=0; //左マウスボタン押下フラグ
static int MouseRB_ON=0; //右マウスボタン押下フラグ

void mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN){
		MouseLB_ON = 1; printf("(%3d,%3d)で左ボタンが押されました\n", x, y);
	}else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP){
		MouseLB_ON = 0; printf("(%3d,%3d)で左ボタンを離しました\n", x, y);
	}else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN){
		MouseRB_ON = 1; printf("(%3d,%3d)で右ボタンが押されました\n", x, y);
	}else if  (button == GLUT_RIGHT_BUTTON && state == GLUT_UP){
		MouseRB_ON = 0; printf("(%3d,%3d)で右ボタンを離しました\n", x, y);
	}
}

void dragMotion(int x, int y)
{
	if (MouseLB_ON == 1) printf("(%3d,%3d)で左ドラッグ中...\n", x, y);
	else if (MouseRB_ON == 1) printf("(%3d,%3d)で右ドラッグ中...\n", x, y);
}

void passiveMotion(int x, int y)
{
	printf("(%3d,%3d)でマウス移動中...\n", x, y);
}

// ウインドウサイズ変更時に呼び出される関数（Reshapeコールバック関数）
void reshape(int w, int h) 
{
	glViewport(0, 0, w, h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(30.0, (double)w/h, 1.0, 100.0); // 透視投影

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(5.0, 5.0, 5.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0); // 視点の設定
}
// 文字列を描画する関数
void render_string(float x, float y, float z, const char* str) {
	glDisable(GL_LIGHTING);
	glColor3f(0.0, 0.0, 0.0);
	glRasterPos3f(x, y, z);
	const char* c = str;
	while (*c) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c++);
	}
	glEnable(GL_LIGHTING);
}

void display(void){            // 描画時に呼び出される関数（Displayコールバック関数)
	glClearColor(0.8, 0.9, 1.0, 1.0); // 画面クリア
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 画面バッファクリア
	glEnable(GL_DEPTH_TEST); // 隠面消去を有効
	// 畑を中心に寄せるための変数設定
	float MovingCube_x = ((float)gridSize_x - 1.0) / 2.0;
	float MovingCube_z = ((float)gridSize_z - 1.0) / 2.0;
	
	for (int i = 0; i < gridSize_x; i++) {
		for (int j = 0; j < gridSize_z; j++) {
			glPushMatrix();
				// 材質
				GLfloat mat0ambi[] = { 0.25,  0.17, 0.10, 1.0 }; // 土
				glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat0ambi); //環境光の反射率を設定
				GLfloat mat0diff[] = { 0.50,  0.35, 0.20, 1.0 }; // 土
				glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat0diff); //拡散光の反射率を設定
				GLfloat mat0spec[] = { 0.0,  0.0, 0.0, 1.0 };    // 土
				glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat0spec); //鏡面光の反射率を設定
				GLfloat mat0shine[] = { 0.00 };                  // 土
				glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat0shine);
				// 場所
				glTranslatef((float)i - MovingCube_x, 0.0, (float)j - MovingCube_z);
				// 物体の描画
				glutSolidCube(1.0);	// 立方体
				// 選択中のマスのまわりにwireを描画する
				if (i == select_x && j == select_z) {
					GLfloat frameDiff[] = { 1.0, 1.0, 1.0, 1.0 };
					glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, frameDiff);
					GLfloat frameAmbi[] = { 1.0, 1.0, 0.0, 1.0 };
					glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, frameAmbi);

					glutWireCube(1.01);    // 土より少し大きい線の箱
				}
				if (grid[i][j].stage != EMPTY) {
					glPushMatrix();
						switch (grid[i][j].cropType) {
							case CROP_WHEAT: {
								GLfloat d[] = { 0.85, 0.75, 0.25, 1.0 };   // 黄色
								glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, d);
								GLfloat a[] = { 0.42, 0.37, 0.12, 1.0 };
								glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, a);
								break;
							}
							case CROP_CARROT: {
								GLfloat d[] = { 0.90, 0.50, 0.15, 1.0 };   // オレンジ
								glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, d);
								GLfloat a[] = { 0.45, 0.25, 0.07, 1.0 };
								glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, a);
								break;
							}
							case CROP_TOMATO: {
								GLfloat d[] = { 0.80, 0.20, 0.20, 1.0 };   // 赤
								glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, d);
								GLfloat a[] = { 0.40, 0.10, 0.10, 1.0 };
								glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, a);
								break;
							}
						}
						switch (grid[i][j].stage) {
							case SEED: {
								glTranslatef(0.0, 0.5, 0.0);
								glScalef(0.2, 0.2, 0.2);
								glutSolidSphere(1.0, 20, 20);
								break;
							}
							case GROWING: {
								glTranslatef(0.0, 0.7, 0.0);
								glScalef(0.1, 0.5, 0.1);
								glutSolidCube(1.0);
								break;
							}
							case MATURE: {
								glTranslatef(0.0, 0.7, 0.0);
								glScalef(0.5, 0.5, 0.5);
								glutSolidSphere(1.0, 20, 20);
								break;
							}
						}
					glPopMatrix();
				}
			glPopMatrix();
		}
	}
	char tmp[256];
	sprintf_s(tmp, sizeof(tmp), "Wheat seed:%d  crop:%d", seedCount[CROP_WHEAT], cropCount[CROP_WHEAT]);
	render_string(2.90, 4.15, 4.0, tmp);
	sprintf_s(tmp, sizeof(tmp), "Carrot seed:%d  crop:%d", seedCount[CROP_CARROT], cropCount[CROP_CARROT]);
	render_string(2.88, 4.10, 4.0, tmp);
	sprintf_s(tmp, sizeof(tmp), "Tomato seed:%d  crop:%d", seedCount[CROP_TOMATO], cropCount[CROP_TOMATO]);
	render_string(2.86, 4.05, 4.0, tmp);
	sprintf_s(tmp, sizeof(tmp), "Selected crop: %d", selectedSeed);
	render_string(2.84, 4.00, 4.0, tmp);
	sprintf_s(tmp, sizeof(tmp), "key to select Wheat: 1");
	render_string(0.00, 2.25, 0.0, tmp);
	sprintf_s(tmp, sizeof(tmp), "key to select Carrot: 2");
	render_string(0.00, 2.10, 0.0, tmp);
	sprintf_s(tmp, sizeof(tmp), "key to select Tomato: 3");
	render_string(0.00, 1.95, 0.0, tmp);
	sprintf_s(tmp, sizeof(tmp), "key to plant: p");
	render_string(0.00, 1.80, 0.0, tmp);
	sprintf_s(tmp, sizeof(tmp), "key to harvest: h");
	render_string(0.00, 1.65, 0.0, tmp);

	glutSwapBuffers(); // 描画実行
}

void lightInit(void)        // 光源の初期設定(まとめて関数にしているだけ)
{
	glEnable(GL_LIGHTING);  // 光源の設定を有効にする
	glEnable(GL_LIGHT0);    // 0番目の光源を有効にする(8個まで設定可能)
	glEnable(GL_NORMALIZE); // 法線ベクトルの自動正規化を有効

	GLfloat light0pos[] = { 0.0, 5.0, 0.0, 1.0 };
	glLightfv(GL_LIGHT0, GL_POSITION, light0pos); //光源0の位置を設定

	GLfloat light0ambi[] = { 0.2, 0.2, 0.2, 1.0 };
	glLightfv(GL_LIGHT0, GL_AMBIENT,  light0ambi); //光源0の環境光の色を設定
	GLfloat light0diff[] = { 0.8, 0.8, 0.8, 1.0 };
	glLightfv(GL_LIGHT0, GL_DIFFUSE,  light0diff); //光源0の拡散光の色を設定
	GLfloat light0spec[] = { 0.5, 0.5, 0.5, 1.0 };
	glLightfv(GL_LIGHT0, GL_SPECULAR, light0spec); //光源0の鏡面光の色を設定

	glShadeModel(GL_SMOOTH); //スムーズシェーディングに設定
}

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);          // GLUT初期化
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(640, 480);   // ウィンドウサイズの指定
	glutCreateWindow("window");     // 表示ウィンドウ作成
	glutReshapeFunc(reshape);       // Reshapeコールバック関数の指定
	glutDisplayFunc(display);       // Displayコールバック関数の指定

	glutMouseFunc(mouse);           // マウスクリックコールバック関数の指定
	glutMotionFunc(dragMotion);     // マウスドラッグコールバック関数の指定
	glutPassiveMotionFunc(passiveMotion);// マウス移動コールバック関数の指定
	glutKeyboardFunc(keyboard);     // 通常キーコールバック関数の指定(押したとき)
	glutKeyboardUpFunc(keyboardUp); // 通常キーコールバック関数の指定(離したとき)
	glutSpecialFunc(specialKey);    // 特殊キーコールバック関数の指定(押したとき)
	glutSpecialUpFunc(specialKeyUp);// 特殊キーコールバック関数の指定(離したとき)
	initGrid();     // 土キューブの初期化
	initRecipes();  // レシピの設定
	initInventory(); 			    // インベントリの初期化
	lightInit();    // 光源の初期設定(まとめて関数にしているだけ)
	glutTimerFunc(1000, timer, 0);
	PlaySound(TEXT("bgm.1_112836.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
	glutMainLoop(); // メインループへ
	return 0;
}
