#include "tetris.h"

static struct sigaction act, oact;

int main(){
	int exit=0;

	initscr();
	noecho();
	keypad(stdscr, TRUE);

	createRankList(); //?

	srand((unsigned int)time(NULL));

	recRoot = (RecNode *)malloc(sizeof(RecNode));
	recRoot->lv = -1;
	recRoot->score = 0;
	recRoot->f = field;
	constructRecTree(recRoot); 

	while(!exit){
		clear();
		switch(menu()){
		case MENU_PLAY: play(); break;
		case MENU_EXIT: exit=1; break;
		case MENU_RANK: rank(); break;
		default: break;
		}
	}

	endwin();
	system("clear");
	return 0;
}

void InitTetris(){
	int i,j;

	for(j=0;j<HEIGHT;j++)
		for(i=0;i<WIDTH;i++)
			field[j][i]=0;

	nextBlock[0]=rand()%7;
	nextBlock[1]=rand()%7;
	nextBlock[2]=rand()%7;
	recommend();
	blockRotate=0;
	blockY=-1;
	blockX=WIDTH/2-2;
	score=0;	
	gameOver=0;
	timed_out=0;

	DrawOutline();
	DrawField();
	DrawBlockWithFeatures(blockY,blockX,nextBlock[0],blockRotate);
	DrawNextBlock(nextBlock);
	PrintScore(score);
}

void DrawOutline(){	
	int i,j;
	/* 블럭이 떨어지는 공간의 태두리를 그린다.*/
	DrawBox(0,0,HEIGHT,WIDTH);

	/* next block을 보여주는 공간의 태두리를 그린다.*/
	move(2,WIDTH+10);
	printw("NEXT BLOCK");
	DrawBox(3,WIDTH+10,4,8);
	DrawBox(9,WIDTH+10,4,8);

	/* score를 보여주는 공간의 태두리를 그린다.*/
	move(15,WIDTH+10);
	printw("SCORE");
	DrawBox(16,WIDTH+10,1,8);
}

int GetCommand(){
	int command;
	command = wgetch(stdscr);
	switch(command){
	case KEY_UP:
		break;
	case KEY_DOWN:
		break;
	case KEY_LEFT:
		break;
	case KEY_RIGHT:
		break;
	case ' ':	/* space key*/
		/*fall block*/
		break;
	case 'q':
	case 'Q':
		command = QUIT;
		break;
	default:
		command = NOTHING;
		break;
	}
	return command;
}

int ProcessCommand(int command){
	int ret=1;
	int drawFlag=0;
	switch(command){
	case QUIT:
		ret = QUIT;
		break;
	case KEY_UP:
		if((drawFlag = CheckToMove(field,nextBlock[0],(blockRotate+1)%4,blockY,blockX)))
			blockRotate=(blockRotate+1)%4;
		break;
	case KEY_DOWN:
		if((drawFlag = CheckToMove(field,nextBlock[0],blockRotate,blockY+1,blockX)))
			blockY++;
		break;
	case KEY_RIGHT:
		if((drawFlag = CheckToMove(field,nextBlock[0],blockRotate,blockY,blockX+1)))
			blockX++;
		break;
	case KEY_LEFT:
		if((drawFlag = CheckToMove(field,nextBlock[0],blockRotate,blockY,blockX-1)))
			blockX--;
		break;
	default:
		break;
	}
	if(drawFlag) DrawChange(field,command,nextBlock[0],blockRotate,blockY,blockX);
	return ret;	
}

void DrawField(){
	int i,j;
	for(j=0;j<HEIGHT;j++){
		move(j+1,1);
		for(i=0;i<WIDTH;i++){
			if(field[j][i]==1){
				attron(A_REVERSE);
				printw(" ");
				attroff(A_REVERSE);
			}
			else printw(".");
		}
	}
}


void PrintScore(int score){
	move(17,WIDTH+11);
	printw("%8d",score);
}

void DrawNextBlock(int *nextBlock){
	int i, j;
	for( i = 0; i < 4; i++ ){
		move(4+i,WIDTH+13);
		for( j = 0; j < 4; j++ ){
			if( block[nextBlock[1]][0][i][j] == 1 ){
				attron(A_REVERSE);
				printw(" ");
				attroff(A_REVERSE);
			}
			else printw(" ");
		}
	}
	for( i = 0; i < 4; i++ ){
		move(10+i, WIDTH+13);
		for( j = 0; j < 4; j++ ){
			if( block[nextBlock[2]][0][i][j] == 1){
				attron(A_REVERSE);
				printw(" ");
				attroff(A_REVERSE);
			}
			else printw(" ");
		}
	}
}

void DrawBlock(int y, int x, int blockID,int blockRotate,char tile){
	int i,j;
	for(i=0;i<4;i++)
		for(j=0;j<4;j++){
			if(block[blockID][blockRotate][i][j]==1 && i+y>=0){
				move(i+y+1,j+x+1);
				attron(A_REVERSE);
				printw("%c",tile);
				attroff(A_REVERSE);
			}
		}

	move(HEIGHT,WIDTH+10);
}

void DrawBox(int y,int x, int height, int width){
	int i,j;
	move(y,x);
	addch(ACS_ULCORNER);
	for(i=0;i<width;i++)
		addch(ACS_HLINE);
	addch(ACS_URCORNER);
	for(j=0;j<height;j++){
		move(y+j+1,x);
		addch(ACS_VLINE);
		move(y+j+1,x+width+1);
		addch(ACS_VLINE);
	}
	move(y+j+1,x);
	addch(ACS_LLCORNER);;
	for(i=0;i<width;i++)
		addch(ACS_HLINE);
	addch(ACS_LRCORNER);
}

void play(){
	int command;
	clear();
	act.sa_handler = BlockDown;
	sigaction(SIGALRM,&act,&oact);
	InitTetris();
	do{
		if(timed_out==0){
			alarm(1);
			timed_out=1;
		}

		command = GetCommand();
		if(ProcessCommand(command)==QUIT){
			alarm(0);
			DrawBox(HEIGHT/2-1,WIDTH/2-5,1,10);
			move(HEIGHT/2,WIDTH/2-4);
			printw("Good-bye!!");
			refresh();
			getch();

			return;
		}
	}while(!gameOver);

	alarm(0);
	getch();
	DrawBox(HEIGHT/2-1,WIDTH/2-5,1,10);
	move(HEIGHT/2,WIDTH/2-4);
	printw("GameOver!!");
	refresh();
	getch();
	newRank(score);
}

char menu(){
	printw("1. play\n");
	printw("2. rank\n");
	printw("3. recommended play\n");
	printw("4. exit\n");
	return wgetch(stdscr);
}

int CheckToMove(char f[HEIGHT][WIDTH],int currentBlock,int blockRotate, int blockY, int blockX){
	// user code
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			if (block[currentBlock][blockRotate][i][j] == 1) {
				if (f[blockY+i][blockX+j]) return 0;
				if (blockY+i >= HEIGHT) return 0;
				if (blockX+j < 0) return 0;
				if (blockX+j >= WIDTH) return 0;
			}
		}
	}
	return 1;
}

void DrawChange(char f[HEIGHT][WIDTH],int command,int currentBlock,int blockRotate, int blockY, int blockX){
	// user code
	int pastX = blockX, pastY = blockY, pastRot = blockRotate, pastShad;
	switch(command) {
	case KEY_LEFT:
		pastX += 1;
		break;
	case KEY_DOWN:
		pastY -= 1;
		break;
	case KEY_RIGHT:
		pastX -= 1;
		break;
	case KEY_UP:
		pastRot = (blockRotate+3) % 4;
		break;
	}
	pastShad = pastY;
	while (CheckToMove(f, currentBlock, pastRot, ++pastShad, pastX));
	pastShad -= 1;
	for (int i = 0; i < 4; i++) {
		for(int j = 0; j < 4; j++) {
			if (block[currentBlock][pastRot][i][j] == 1) {
				if (pastY+i>=0) {
					move(pastY+1+i, pastX+1+j);
					printw(".");
				}
				if (i + pastShad >= 0) {
					move(i+pastShad+1, j+pastX+1);
					printw(".");
				}
			}
		}
	}
	DrawBlockWithFeatures(blockY,blockX,currentBlock,blockRotate);
	move(HEIGHT, WIDTH+10);
}

void BlockDown(int sig){
	// user code
	if(CheckToMove(field, nextBlock[0], blockRotate, blockY+1, blockX)) {
		blockY++;
		DrawChange(field, KEY_DOWN, nextBlock[0], blockRotate, blockY, blockX);
	}
	else {
		if(blockY == -1) gameOver = 1;
		score += AddBlockToField(field,nextBlock[0],blockRotate,blockY,blockX);
		score += DeleteLine(field);
		blockRotate = 0;
		blockY = -1;
		blockX = (WIDTH/2) - 2;
		nextBlock[0] = nextBlock[1]; 
		nextBlock[1] = nextBlock[2];
		nextBlock[2]= rand() % 7;
		recommend();
		DrawNextBlock(nextBlock);
		PrintScore(score);
		DrawField();
		DrawBlockWithFeatures(blockY, blockX, nextBlock[0], blockRotate);
	}	
	timed_out=0;
	return;
}

int AddBlockToField(char f[HEIGHT][WIDTH],int currentBlock,int blockRotate, int blockY, int blockX){
	// user code
	int touched = 0;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			if (block[currentBlock][blockRotate][i][j] == 1) {
				f[blockY+i][blockX+j] = 1;
				if (blockY+1+i==HEIGHT || f[blockY+1+i][blockX+j] == 1)
					touched += 1;
			}
		}
	}
	return touched*10;
}

int DeleteLine(char f[HEIGHT][WIDTH]){
	// user code
	int flag;
	int lineNum = 0;
	for (int i = 0; i < HEIGHT; i++) {
		flag = 1;
		for (int j = 0; j < WIDTH; j++) {
			if (f[i][j] == 0) {
				flag = 0;
				break;
			}
		}
		if (flag == 1) {
			lineNum++;
			for (int j = i; j > 0; j--) {
				for(int k = 0; k < WIDTH; k++)
				f[j][k] = f[j-1][k];
			}
		}
	}
	return 100*lineNum*lineNum;
}

void DrawShadow(int y, int x, int blockID,int blockRotate){
	// user code
	while(CheckToMove(field, blockID, blockRotate, ++y, x));
	DrawBlock(--y, x, blockID, blockRotate, '/');
}

void DrawRecommend() {
	if (CheckToMove(field, nextBlock[0], recommendR, recommendY, recommendX) == 1)
		DrawBlock(recommendY, recommendX, nextBlock[0], recommendR, 'R');
}

void createRankList(){
	// user code
	FILE * fp;
	int num, scoretmp;
	char str[51], nametmp[NAMELEN];
	head = NULL;
	
	fp = fopen("rank.txt", "r");
	if (fp == NULL) {
		printw("The file is empty");
		return;
	}	
	fscanf(fp, "%d", &user_num);
	fgetc(fp);
	if (user_num <= 0) {
		printw("The number of info should be positive integer");
		return;
	}
	for (int i = 0; i < user_num; i++) {
		fscanf(fp, "%s %d\n", nametmp, &scoretmp);
		insert(nametmp, scoretmp);
	}
	fclose(fp);
}

void rank(){
	// user codei
	int from = 1, to, loop, i;
	int del;
	int flag = 0;
	char name[NAMELEN];
	nptr tmp;
	nptr tmp2;
	clear();
	move(0, 0);
	printw("1. list ranks from X to Y\n");
	printw("2. list ranks by a specific name\n");
	printw("3. delete a specific rank\n");
	switch(wgetch(stdscr)) {
		case '1':
			//createRankList();
			to = user_num;
			echo();
			printw("X: ");
			scanw("%d", &from);
			printw("Y: ");
			scanw("%d", &to);
			
			noecho();
			printw("       name       |   score   \n");
			printw("------------------------------\n");
			if (from < 0 || to < 0) {
				printw("search failure: no rank in the list\n");
				break;
			}
			else if (from > to) {
				printw("search failure: no rank in the list\n");
				break;
			}
			else if (from == 0 && to == 0) {
				tmp = head;
				while (tmp != NULL) {
					printw("%10s\t\t%8d\n", tmp->name, tmp->score);
					tmp = tmp->link;
				}
			}
			else {
				tmp = head;
				for (i = 1; i < from; i++) {
					tmp = tmp->link;
				}
				for (i = from; i <= to; i++) {
					printw("%10s\t\t%8d\n", tmp->name, tmp->score);
					tmp = tmp->link;
				}
			}
			break;
		case '2':
			printw("input the name: ");
			echo();
			getstr(name);
			noecho();
			printw("       name       |   score   \n");
			printw("------------------------------\n");
			tmp = head;
			while(tmp != NULL) {
				if (strcmp(name, tmp->name) == 0) {
					printw("%10s\t\t%8d\n", tmp->name, tmp->score);
					flag = 1;
				}
				tmp = tmp->link;
			}
			if (flag == 0) printw("\nsearch failure: no name in the list\n");
			//printw("\nresult: the rank deleted\n");

			break;
		case '3':
			printw("input the rank: ");
			echo();
			scanw("%d",&del);
			noecho();
			if (del <= 0) {
				printw("\nsearch failure: the rank not in the list\n");
				break;
			}
			if (head == NULL) {
				printw("\nsearch failure: the rank not in the list\n");
				break;
			}
			tmp = head;
			if (del == 1) {
				tmp2 = head;
				head = tmp->link;
				free(tmp2);
				printw("\nresult: the rank deleted\n");
				user_num--;
				writeRankFile();
				break;
			}
			for(i = 1; i < del-1; i++) {
				if (tmp->link == NULL) break;
				tmp = tmp->link;
			}
			if (tmp->link == NULL) {
				printw("\nsearch failure: the rank not in the list\n");
				break;
			}
			tmp2 = tmp->link;
			tmp->link = tmp->link->link;
			free(tmp2);
			printw("\nresult: the rank deleted\n");
			user_num--;
			writeRankFile();
			break;
		default: break;
	}
	getch();
	// user code
}
			
void writeRankFile(){
	// user code
	FILE *fp;
	nptr tmp;
	//int i = 0;
	//if ((fp = fopen("rank.txt", "r")) != NULL) {
	//	fscanf(fp, "%d", &i);
	//}
	tmp = head;
	fp = fopen("rank.txt", "w");
	fprintf(fp, "%d\n", user_num);
	while( tmp!=NULL) {
		fprintf(fp, "%s %d\n", tmp->name, tmp->score);
		tmp = tmp->link;
	}
	fclose(fp);
}

void newRank(int score){
	// user code
	char nameTemp[NAMELEN];
	clear();
	printw("your name: ");
	echo();
	getstr(nameTemp);
	noecho();

	insert(nameTemp, score);
	
	user_num++;
	writeRankFile();
}

int recommend(){
	int ret; // 미리 보이는 블럭의 추천 배치까지 고려했을 때 얻을 수 있는 최대 점수

	// user code
	ret = recommendUsingTree(recRoot); // RecTree 이용
	return ret;
}

void recommendedPlay(){
	// user code
}

void DrawBlockWithFeatures(int y, int x, int blockID, int blockRotate) {
	DrawRecommend();
	DrawShadow(y, x, blockID, blockRotate);
	DrawBlock(y, x, blockID, blockRotate, ' ');
}

void insert(char nametmp[20], int scoretmp) {
	nptr new, tmp, ltmp;

	new = (nptr)malloc(sizeof(Node));	
	for(int i = 0; i < NAMELEN; i++) new->name[i] = nametmp[i];
	new->score = scoretmp;

	tmp = head;
	
	if (tmp == NULL) head = new;
	else {
		if (tmp->score <= new->score) {
			new->link = head;
			head = new;
		}
		else {
			while(tmp->link != NULL && tmp->link->score >= scoretmp)
				tmp = tmp->link;
			new->link = tmp->link;
			tmp->link = new;
		}
	}
}

void constructRecTree(RecNode *root) {
	int i, h;
	RecNode **c = root->c;
	for (i = 0; i < CHILDREN_MAX; ++i) {//일단 최대(한층에336) 개수만큼 만듦
		c[i] = (RecNode *)malloc(sizeof(RecNode));
		c[i]->lv = root->lv + 1;
		c[i]->f = (char(*)[WIDTH])malloc(sizeof(char)*HEIGHT*WIDTH);
		if (c[i]->lv < VISIBLE_BLOCKS) {//몇개까지볼지
			constructRecTree(c[i]);
		}
	}
}

void destructRecTree(RecNode *root) {
	int i, h;
	RecNode **c = root->c;
	for (i = 0; i < CHILDREN_MAX; ++i) {
		if (c[i]->lv < VISIBLE_BLOCKS) {
			destructRecTree(c[i]);
		}
		free(c[i]->f);
		free(c[i]);
	}
}

int evalState(int lv, char f[HEIGHT][WIDTH], int r, int y, int x) {//채우고 지
	return AddBlockToField(f, nextBlock[lv], r, y, x) + DeleteLine(f);
}

int recommendUsingTree(RecNode *root) {
	int r, x, y, rBoundary, lBoundary;
	int h, w;
	int eval;
	int max = 0;
	int solR, solY, solX;
	int recommended = 0;
	int i = 0;
	int lv = root->lv + 1;
	RecNode **c = root->c;
	
	//이것만 하면 됩니다. 트리 탐색하면서 점수를 계산해서 가장 큰 점수가 되는 rotation, x좌표, y좌표 구하기
	//DFS
	//최고 점수 return
	for( r = 0; r <NUM_OF_ROTATE; r++) {
		rBoundary = 0;
		lBoundary = 3;
		for(h = 0; h <BLOCK_HEIGHT; h++) {
			for(w=BLOCK_WIDTH-1; w>=0; w--)
				if (block[nextBlock[lv]][r][h][w]) break;
			if(w>rBoundary) rBoundary=w;
		}
		rBoundary=WIDTH-1-rBoundary;

		for(h=0; h<BLOCK_HEIGHT; h++) {
			for(w=0; w<BLOCK_WIDTH; w++) {
				if (block[nextBlock[lv]][r][h][w]) break;
			if(w<lBoundary) lBoundary=w;
			}
		}
		lBoundary = 0-lBoundary;
	
		for(x=lBoundary; x<=rBoundary; x++, i++) {
			for(h=0;h<HEIGHT;h++)
				for(w=0;w<WIDTH;w++) c[i]->f[h][w]=root->f[h][w];
			y=0;
			if(CheckToMove(c[i]->f, nextBlock[lv], r, y, x)){
				while(CheckToMove(c[i]->f, nextBlock[lv], r, ++y, x));
				 y--;
			}
			else continue;
			c[i]->score=root->score+evalState(lv, c[i]->f, r, y, x);
			if(lv < VISIBLE_BLOCKS-1) eval=recommendUsingTree(c[i]);
			else eval=c[i]->score;
			if(max<eval) {
				recommended=1;
				max = eval;
				solR = r;
				solY = y;
				solX = x;
			}
		}
	}
	if (lv==0 && recommended) {
		recommendR = solR;
		recommendY = solY;
		recommendX = solX;
	}
	return max;
}
