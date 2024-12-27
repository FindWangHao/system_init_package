#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <termios.h>
#include <unistd.h>

// 修改终端的控制方式，1取消回显、确认 ２获取数据 3还原
static int getch(void){
    // 记录终端的配置信息
    struct termios _old;
    // 获取终端的配置信息
    tcgetattr(STDIN_FILENO,&_old);
    // 设置新的终端配置
    struct termios _new = _old;
    // 取消确认、回显
    _new.c_lflag &= ~(ICANON|ECHO);
    // 设置终端配置信息
    tcsetattr(STDIN_FILENO,TCSANOW,&_new);
    // 在新模式下获取数据
    int key_val = 0;
    do{
        key_val += getchar();
    }while(stdin->_IO_read_end - stdin->_IO_read_ptr);
    // 还原配置信息
    tcsetattr(STDIN_FILENO,TCSANOW,&_old);
    return key_val;
}


unsigned short arr[4][4];
unsigned long score = 0;
bool is_rand;
// 统计空位置
bool is_null(void){
	for(int r=0; r<4; r++){
		for(int c=0; c<4; c++){
			if(0 == arr[r][c]){
				return true;
			}
		}
	}
	return false;
}
void rand_num(void){
	// 如果没有空位置则退出
	if(!is_null()){
		return;
	}
	// is_rand为真时才随机产生数字
	while(is_rand){
		int r = rand() % 4;
		int c = rand() % 4;
		if(0 == arr[r][c]){
			arr[r][c] = (rand()%10 < 8) ? 2 : 4;
			return;
		}
	}
}
void init_game(void){
	// 初始化随机数
	srand(time(NULL));
	// 数组清空
	for(int r=0; r<4; r++){
		for(int c=0; c<4; c++){
			arr[r][c] = 0;
		}
	}
	// 允许产生随机数
	is_rand = true;
	// 产生两个随机数
	rand_num();
	rand_num();
}
void show_arr(void){
	system("clear");
	printf("SCORE: %lu\n",score);
	for(int r=0; r<4; r++){
		printf("---------------------\n");
		for(int c=0; c<4; c++){
			switch(arr[r][c]){
				case 0: 
					printf("|    ");
					break;
				case 2: case 4: case 8:
					printf("| %d  ",arr[r][c]);
					break;
				case 16: case 32: case 64:
					printf("| %d ",arr[r][c]);
					break;
				case 128: case 256: case 512:
					printf("| %d",arr[r][c]);
					break;
				case 1024: case 2048: case 4096: case 8192:
					printf("|%d",arr[r][c]);
					break;
				default:
					printf("|xxxx");
					break;
			}
		}
		printf("|\n");
	}
	printf("---------------------\n");
}
void up_move(void){
	// 按列处理
	for(int c=0; c<4; c++){
		// 从下标为1的行向上开始移动
		for(int r=1; r<4; r++){
			// 空位置不需要移动
			if(0 == arr[r][c]){
				continue;
			}
			// 向上寻找最终移动位置
			int end = -1;
			for(int m=r-1; m>=0; m--){
				if(arr[m][c] == arr[r][c] || 0 == arr[m][c]){
					end = m;
				}else{
					break;
				}
			}
			// 找到了最终的移动位置
			if(-1 != end){
				// 计算得分
				score += arr[end][c]*2;
				arr[end][c] += arr[r][c];
				arr[r][c] = 0;
				is_rand = true;	
			}
		}
	}
}
void down_move(void){
	for(int c=0; c<4; c++){
		// 从下标为3的行向下开始移动
		for(int r=3; r>=0; r--){
			// 空位置不需要移动
			if(0 == arr[r][c]){
				continue;
			}
			// 向下寻找最终移动位置
			int end = -1;
			for(int m=r+1; m<4; m++){
				if(arr[m][c] == arr[r][c] || 0 == arr[m][c]){
					end = m;
				}else{
					break;
				}
			}
			// 找到了最终的移动位置
			if(-1 != end){
				// 计算得分
				score += arr[end][c]*2;
				arr[end][c] += arr[r][c];
				arr[r][c] = 0;
				is_rand = true;	
			}
		}
	}
}
void right_move(void){
	//按行处理
	for(int r=0; r<4; r++){
		// 从下标为3的行向左开始移动
		for(int c=3; c>=0; c--){
			// 空位置不需要移动
			if(0 == arr[r][c]){
				continue;
			}
			// 向左寻找最终移动位置
			int end = -1;
			for(int m=c+1; m<4; m++){
				if(arr[r][m] == arr[r][c] || 0 == arr[r][m]){
					end = m;
				}else{
					break;
				}
			}
			// 找到了最终的移动位置
			if(-1 != end){
				// 计算得分
				score += arr[r][end]*2;
				arr[r][end] += arr[r][c];
				arr[r][c] = 0;
				is_rand = true;	
			}
		}
	}
} 
void left_move(void){
	//按行处理
	for(int r=0; r<4; r++){
		// 从下标为3的行向左开始移动
		for(int c=1; c<4; c++){
			// 空位置不需要移动
			if(0 == arr[r][c]){
				continue;
			}
			// 向左寻找最终移动位置
			int end = -1;
			for(int m=c-1; m>=0; m--){
				if(arr[r][m] == arr[r][c] || 0 == arr[r][m]){
					end = m;
				}else{
					break;
				}
			}
			// 找到了最终的移动位置
			if(-1 != end){
				// 计算得分
				score += arr[r][end]*2;
				arr[r][end] += arr[r][c];
				arr[r][c] = 0;
				is_rand = true;	
			}
		}
	}
} 
bool check_over(void){
	if(is_null()){
		return false;
	}else{
		return true;
	}
}
int main(int argc,const char* argv[]){
	init_game();
	for(;;){
		show_arr();
		is_rand = false;
		switch(getch()){
			case 183: 
				up_move();
				break;
			case 184:
				down_move();
				break;
			case 185:
				right_move();
				break;
			case 186:
				left_move();
				break;
			case 'n':
				init_game();
				break;
			case 'q':
				return 0;
		}
		rand_num();
		if(check_over()){
			show_arr();
			printf("游戏结束\n");
			break;
		}
	}
}
