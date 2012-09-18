#include <iostream>
#include <vector>
#include <string.h>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
using namespace std;

vector<char*> words;

void print_words();

struct params{
	vector<char*>& pth_words;
	params(vector<char*>& v):pth_words(v){}
};

bool sort_func(const char* a, const char* b){
	return strcmp(a, b) < 0;
}

void* sort_merge(void* args){
	params* p = (params*)args;
	sort(p->pth_words.begin(), p->pth_words.end(), sort_func);
	return NULL;
}

int main(int argc, char* argv[]){
	timeval start, finish;
	if(argc != 3){
		printf("wrong arguement\n");
		return 1;
	}
	int i, t_number, j;
	char word[100];
	
	if(!sscanf(argv[1], "%d", &t_number)){
		printf("Wrong arguement thread_number: %s\n", argv[1]);
		return 1;
	}
	printf("loading content of words...");
	//begin to timer
	words.reserve(300000);
	gettimeofday(&start, 0);
	FILE* fp = fopen(argv[2], "r");
	while(fscanf(fp, "%s", word) != EOF){
		char* tmp = new char[strlen(word)+1];
		//printf("..%2d..%s\n", strlen(word), word);
		strcpy(tmp, word);
		words.push_back(tmp);
	}
	
	gettimeofday(&finish, 0);
	long long unsigned diff = 1000000*(finish.tv_sec-start.tv_sec)+finish.tv_usec-start.tv_usec;
	printf("%d words, %llu ms.\n", words.size(), diff/1000);
	gettimeofday(&start, 0);

	int word_count = words.size();
	int word_count_div = word_count/t_number;
	int word_count_mod = word_count%t_number;
	printf("div:%d\tmod:%d\n", word_count_div, word_count_mod);
	
	vector<vector<char*> > th_words(t_number);
	pthread_t* vths = new pthread_t [t_number];
	word_count = 0;
	struct params** pm = new params* [t_number];
	for(i = 0; i < t_number; ++i){
		if(i < word_count_mod){
			th_words[i].resize(word_count_div+1);
		}else{
			th_words[i].resize(word_count_div);
		}
		for(j = 0; j < th_words[i].size(); ++j){
			th_words[i][j] = words[word_count++];
		}
		pm[i] = new struct params(th_words[i]);
		
		if(pthread_create(&vths[i], NULL, sort_merge, pm[i])){
			printf("thread create error\n");
			return 1;
		}
	}
	
	for(i = 0; i < t_number; ++i){
		pthread_join(vths[i], NULL);
	}
	printf("thread sort complete.");
	gettimeofday(&finish, 0);
	diff = 1000000*(finish.tv_sec-start.tv_sec)+finish.tv_usec-start.tv_usec;
	printf("..%llu ms.\n", diff/1000);
	gettimeofday(&start, 0);
	int *idx = new int [t_number], row_x;
	int *counted = new int [t_number];
	memset(idx, 0, t_number*sizeof(int));
	memset(counted, 0, t_number*sizeof(int));

	j = 0;
	char* min;
	word_count = 0;
	while(j < t_number){
		for(i = 0; i < t_number; ++i){
			if(idx[i] < th_words[i].size()){
				min = th_words[i][idx[i]];
				break;
			}else if(!counted[i]){
				counted[i] = 1;
				++j;
			}
		}
		row_x = i;
		for(++i; i < t_number; ++i){
			if(idx[i] == th_words[i].size()){
				if(0 == counted[i]){
					counted[i] = 1;
					++j;
				}
				continue;
			}
			if(strcmp(th_words[i][idx[i]], min) < 0){
				min = th_words[i][idx[i]];
				row_x = i;
			}
		}
		if(j == t_number){
			break;
		}
		++idx[row_x];
		words[word_count++] = min;
	}

	delete [] idx;
	delete [] counted;

	gettimeofday(&finish, 0);
	diff = 1000000*(finish.tv_sec-start.tv_sec)+finish.tv_usec-start.tv_usec;
	printf("sort consumes %llu ms.\n", diff/1000);
	/*print all sorted words*/
	timeval finish2;
	printf("printing out words...\n");
	print_words();
	gettimeofday(&finish2, 0);
	diff = 1000000*(finish2.tv_sec-finish.tv_sec)+finish2.tv_usec-finish.tv_usec;
	printf("print out consumes %llu ms.\n", diff/1000);
	return 0;
}

void print_words(){
	int i;
	FILE* fop = fopen("word.out", "w");
	for(i = 0; i < words.size(); ++i){
		fprintf(fop,"%s\n", words[i]);
	}
}
