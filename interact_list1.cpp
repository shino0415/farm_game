#include <stdio.h>
#include <glut.h>

// 配列の箱の最大
const int MAX_SIZE = 9;
// 今使ってる範囲
int gridSize_x = 3;
int gridSize_z = 3;

// 選択しているマス
int select_x = 1;
int select_z = 1;

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
};

// 土のマス目
Cell grid[MAX_SIZE][MAX_SIZE];
void initGrid(void) {
	for (int i = 0; i < MAX_SIZE; i++) {
		for (int j = 0; j < MAX_SIZE; j++) {
			grid[i][j].stage = EMPTY;
			grid[i][j].cropType = CROP_NONE;
		}
	}
}

void timer(int value)
{
	for (int i = 0; i < gridSize_x; i++) {
		for (int j = 0; j < gridSize_z; j++) {
			// 作物の成長
			if (grid[i][j].stage == SEED) {
				grid[i][j].stage = GROWING;
			}
			else if (grid[i][j].stage == GROWING) {
				grid[i][j].stage = MATURE;
			}
		}
	}
	glutPostRedisplay();
	glutTimerFunc(1000, timer, 0);
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key) {
		case 'p':
			printf("(%3d,%3d)でpが押されました\n", x, y);
			if (grid[select_x][select_z].stage == EMPTY) {
				grid[select_x][select_z].stage = SEED;
				grid[select_x][select_z].cropType = CROP_WHEAT;
			}
			break;
		case '1':
			printf("(%3d,%3d)で1が押されました\n", x, y);
			break;
		case '2':
			printf("(%3d,%3d)で2が押されました\n", x, y);
			break;
		case '3':
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

void display(void){            // 描画時に呼び出される関数（Displayコールバック関数)
	glClearColor(0.0, 0.0, 0.0, 1.0); // 画面クリア
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 画面バッファクリア
	glEnable(GL_DEPTH_TEST); // 隠面消去を有効
	// 畑を中心に寄せるための変数設定
	float MovingCube_x = ((float)gridSize_x - 1.0) / 2.0;
	float MovingCube_z = ((float)gridSize_z - 1.0) / 2.0;
	for (int i = 0; i < gridSize_x; i++) {
		for (int j = 0; j < gridSize_z; j++) {
			// 材質
			switch (grid[i][j].stage) {
				case EMPTY: {
					GLfloat mat0ambi[] = { 0.25,  0.17, 0.10, 1.0 }; // 土
					glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat0ambi); //環境光の反射率を設定
					GLfloat mat0diff[] = { 0.50,  0.35, 0.20, 1.0 }; // 土
					glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat0diff); //拡散光の反射率を設定
					break;
				}
				case SEED: {
					GLfloat mat0ambi[] = { 0.27,  0.22, 0.10, 1.0 }; // 土
					glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat0ambi); //環境光の反射率を設定
					GLfloat mat0diff[] = { 0.55,  0.45, 0.20, 1.0 }; // 土
					glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat0diff); //拡散光の反射率を設定
					break;
				}
				case GROWING: {
					GLfloat mat0ambi[] = { 0.32,  0.27, 0.15, 1.0 }; // 土
					glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat0ambi); //環境光の反射率を設定
					GLfloat mat0diff[] = { 0.65,  0.55, 0.30, 1.0 }; // 土
					glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat0diff); //拡散光の反射率を設定
					break;
				}
				case MATURE: {
					GLfloat mat0ambi[] = { 0.37,  0.32, 0.25, 1.0 }; // 土
					glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat0ambi); //環境光の反射率を設定
					GLfloat mat0diff[] = { 0.75,  0.65, 0.50, 1.0 }; // 土
					glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat0diff); //拡散光の反射率を設定
					break;
				}
			}
			GLfloat mat0spec[] = { 0.0,  0.0, 0.0, 1.0 };    // 土
			glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat0spec); //鏡面光の反射率を設定
			GLfloat mat0shine[] = { 0.00 };                  // 土
			glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat0shine);
			glPushMatrix();
				// 場所
				glTranslatef((float)i - MovingCube_x, 0.0, (float)j - MovingCube_z);
				// 物体の描画
				glutSolidCube(1.0);	// 立方体
				// 選択中のマスをデフォルメ
				if (i == select_x && j == select_z) {
					GLfloat frameDiff[] = { 1.0, 1.0, 1.0, 1.0 };
					glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, frameDiff);
					GLfloat frameAmbi[] = { 1.0, 1.0, 0.0, 1.0 };
					glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, frameAmbi);

					glutWireCube(1.01);    // 土より少し大きい線の箱
				}
			glPopMatrix();
		}
	}
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
	lightInit();    // 光源の初期設定(まとめて関数にしているだけ)
	glutTimerFunc(1000, timer, 0);
	glutMainLoop(); // メインループへ
	return 0;
}
