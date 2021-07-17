#include<iostream>
#include<string.h>
#include<fstream>
#include<list>
#include<cmath>
#include<vector>
#include<utility>
using namespace std;

struct cacheBlock{
	int tag;
	int validbit;
	int dirtybit;
};


typedef struct cacheBlock block;
vector<list<block>> cache;
vector<vector<block>> tree;
vector<pair<int,int>> evicted_array;
int evictedCount=0;
int cacheMisses=0;
int compulsaryMisses=0;

int* binary(char ch){//converts a hexa decimal digit into four binary digit array
	static int p[4];
	if(ch>='0'&&ch<='9'){//if ch is from a to f then it is equivalent to 10 to 15
		int x=int(ch);
		int i=3;
		while(x!=0){
			p[i]=x%2;
			x=x/2;
			i--;
		}
		for(int j=i;j>=0;j--)
		     p[j]=0;
	}
	else{
		ch=ch-'W';
		int x=int(ch);
		int i=3;
		while(x!=0){
			p[i]=x%2;
			x=x/2;
			i--;
		}
	}
	return p;
}

int find_block(int index,int tag){//we use find block to find the block in evicted array
     int i;
     pair<int,int> p;
     for(i=0;i<evicted_array.size();i++){
         p=evicted_array[i];
         if(p.first==index&&p.second==tag)
             return 0;
     }
     return 1;
}

void update(int index,int tag){//update is used to update the evicted array where we store the evicted addresses
    int i;
    pair<int,int> p1,p2;
    p1.first=index;
    p1.second=tag;
    for(i=0;i<evicted_array.size();i++){//first we check whether the address is present is already evicted or not
         p2=evicted_array[i];//so that we do not have repeated blocks in the array
         if(p2.first==index&&p2.second==tag)
             return;
    }
    evicted_array.resize(evicted_array.size()+1);//if it is not present then we insert in the array.
    evicted_array.push_back(p1);
}


vector<int> find_bits(int cache_size,int block_size,int associativity){//with cache size block size and associativity we find no of bits
	vector<int> v;//for tag ,index,block offset
	v.resize(3);
	v[0]=ceil(log2(block_size));
	if(associativity==0)
		v[1]=0;
	else
		v[1]=ceil(log2(cache_size/(associativity*block_size)));
	v[2]=32-v[0]-v[1];
	return v;
}

vector<long long int> Values(int bin[32],int cache_size,int block_size,int associativity){//bin[32] has 32 bit converted address stored in array
	vector<long long int> v;//v stores the value of index ,block offset,tag, and value of address in decimal system
	vector<int> p;
	v.resize(4);
	p=find_bits(cache_size,block_size,associativity);
	int i;
	v[0]=0;
	v[1]=0;
	v[2]=0;
	v[3]=0;
	for(i=31;i>31-p[0];i--){//we use the number of bits for each tag index block offset to find the value in decimal representation
		v[0]+=pow(2,31-i)*bin[i];
	}
	for(i=31-p[0];i>31-p[1]-p[0];i--){
		v[1]+=pow(2,31-p[0]-i)*bin[i];
	}
	for(i=31-p[0]-p[1];i>=0;i--){
		v[2]+=pow(2,31-p[0]-p[1]-i)*bin[i];
	}
	for(i=31;i>=0;i--){
		v[3]+=pow(2,31-i)*bin[i];
	}
	return v;
}

int Random_policy(int index,int tag,int ways,int rdwr){//takes the input as tag ,index,no of ways,read or write bit
    auto it=cache[index].begin();//cache is a vector of lists with vector differentiated by index value and each list is differentiated by tag
    block temp;
    temp.tag=tag;
    temp.validbit=1;
    temp.dirtybit=rdwr;//we store all the value into block temp
    int state;
    if(cache[index].size() < ways){//first we search the list to find if the block is already present in the cache or not
    	for(it=cache[index].begin();it!=cache[index].end();it++){
    	    if((*it).tag==tag)
    		break;
     	}
        if(it==cache[index].end()){//if we do not fill the list then we write the block into the list at the end
            state=find_block(index,tag);//we check whether the block is first time accessed or not
            compulsaryMisses+=state;//if it is then we increment the compulsory misses by 1
            cache[index].insert(it,temp);
            cacheMisses++;//and also the cache misses
            return 1;//here return 1 is used to find whether it is read miss or write miss
        }
        else{
            (*it).dirtybit=((*it).dirtybit||rdwr);//the dirty bit is or of the rdwr request and dirty bit of old block
            return 0;
        }
    }
    else{
    	for(it=cache[index].begin();it!=cache[index].end();it++){
    	    if((*it).tag==tag)
    		break;
     	}
        if(it==cache[index].end()){//if there are no empty spaces then if we find the block then no problem same as above
            cacheMisses++;
            it--;
            (*it).validbit=0;
            if((*it).dirtybit==1)//but if we do not find the block then we remove the last block  and push the new block n the front
                evictedCount++;
            update(index,(*it).tag);//now we check whether to write the evicted block into the array or not
            cache[index].erase(it);
            it=cache[index].begin();
            temp.tag=tag;
            temp.validbit=1;
            temp.dirtybit=rdwr;
            state=find_block(index,tag);//and check whether the new block has  compulsory miss or not
            compulsaryMisses+=state;
            cache[index].insert(it,temp);
            return 1;
	}
	else{
            (*it).dirtybit=((*it).dirtybit||rdwr);
            return 0;
        }
    }
}

int LRU_policy(int index,int tag,int ways,int rdwr){
    block temp;
    block temp1;
    auto it=cache[index].begin();
    int state;
    for(it=cache[index].begin();it!=cache[index].end();it++){//find the block whether it is already present in the cache or not
        temp=*it;
        if(temp.tag==tag){//if found then update the dirty bit and erase it at the positin and keep it at first
            temp1.tag=temp.tag;
            temp1.validbit=temp.validbit;
            temp1.dirtybit=(temp.dirtybit||rdwr);
            cache[index].erase(it);
            cache[index].insert(cache[index].begin(),temp1);
            return 0;
        }
    }
    cacheMisses++;//if not found
    if(cache[index].size()<ways){//and there are empty spaces
        temp1.tag=tag;
        temp1.validbit=1;
        temp1.dirtybit=rdwr;
        state=find_block(index,tag);
        compulsaryMisses+=state;//we check whether the miss is compulsory or not and we insert at the begin
        cache[index].insert(cache[index].begin(),temp1);
        return 1;
    }
    it--;//if there are no empty ways then we evict the last block and keep the new one at first
    temp1.tag=tag;
    temp1.validbit=1;
    temp1.dirtybit=rdwr;
    (*it).validbit=0;
    if((*it).dirtybit==1)//while evicting if it is dirty then dirty bit count evicted increses
       evictedCount++;
    update(index,(*it).tag);//update the evicted array using the evicted block
    cache[index].erase(it);
    state=find_block(index,tag);
    compulsaryMisses+=state;//check whether the inserted block is compulsory miss or not
    cache[index].insert(cache[index].begin(),temp1);
    return 1;
}

int pseudo_LRU(int index,int tag,int ways,int rdwr){
	int n=ways;
	int j;
	int parent;
	block temp;
	int state;
	for(int i=n-1;i<2*n-1;i++){//first check whether the block is present in the child nodes
	    if(tree[index][i].tag==tag){//if present then update it and then change the parentnodes which lead to the block
		tree[index][i].dirtybit=(tree[index][i].dirtybit||rdwr);
		tree[index][i].validbit=1;
                j=i;
	        while(j>0){
		    parent=floor((j-1)/2);
		    if(j==2*parent+1)
			tree[index][parent].tag=1;
		    if(j==2*parent+2)
			tree[index][parent].tag=0;
		    j=parent;
	       }
             return 0;
	    }
	}
	int i=0;
	cacheMisses++;
	while(i<n-1){//if not found follow the tree nodes and go to the index in the tree
		if(tree[index][i].tag==0)
			i=2*i+1;
		if(tree[index][i].tag==1)
			i=2*i+2;

	}
	if(tree[index][i].tag!=-1){//if it does not have -1 then it means it has a block already so evict the block and then replace it with new block
	        update(index,tree[index][i].tag);//update the array with the evicted block
		if(tree[index][i].dirtybit==1){
			evictedCount++;
		}
		state=find_block(index,tag);//now check whether the new block has compulsory miss or not
		compulsaryMisses+=state;
		tree[index][i].dirtybit=rdwr;
		tree[index][i].tag=tag;
		tree[index][i].validbit=1;
		j=i;
		while(j>0){//and change the values of the parent nodes
			j=floor((j-1)/2);
			if(tree[index][j].tag==1)
				tree[index][j].tag=0;
			if(tree[index][j].tag==0)
				tree[index][j].tag=1;
		}
		return 1;
	}
	        state=find_block(index,tag);//if it is empty then keep the block and check whether it has compulsory miss or not
	        compulsaryMisses+=state;
		tree[index][i].dirtybit=rdwr;
		tree[index][i].tag=tag;
		tree[index][i].validbit=1;
		j=i;
		while(j>0){//now change the parent node values
			j=floor((j-1)/2);
			if(tree[index][j].tag==1)
				tree[index][j].tag=0;
			if(tree[index][j].tag==0)
				tree[index][j].tag=1;
		}
		return 1;
}

int main(){
	int cache_size;
	int block_size;
	int associativity;
	int policy;

	int cacheAccess=0;
	int readAccess=0;
	int writeAccess=0;
	int readMisses=0;
	int writeMisses=0;

	int capacityMisses=0;
	int conflictMisses=0;

    cout<<"Enter the Cache Size: ";
	cin>>cache_size;
    cout<<"Enter the Block Size: ";
	cin>>block_size;
    cout<<"Enter the Associativity: ";
	cin>>associativity;
    cout<<"Enter the Policy: ";
	cin>>policy;

	int* arr;
	int bin[32];
	int i;
	string add;
	vector<long long int> temp;
	vector<int> p=find_bits(cache_size,block_size,associativity);

	cache.resize(pow(2,p[1]));//this cache is used for LRU and  random implementation
	int ways=cache_size/(block_size*pow(2,p[1]));


	tree.resize(pow(2,p[1]));//the tree is used for PSEUDO LRU
    for(int i=0;i<tree.size();i++){
		tree[i].resize(2*ways-1);
		for(int j=0;j<ways-1;j++){
			tree[i][j].tag=0;
			tree[i][j].validbit=-1;
			tree[i][j].dirtybit=-1;
		}
		for(int j=ways-1;j<2*ways-1;j++){
			tree[i][j].tag=-1;
			tree[i][j].validbit=-1;
			tree[i][j].dirtybit=-1;
		}
	}

	int count;
	ifstream myReadFile("input.txt");

	char add1[12];
	int len;
	int rdwr;

	while(getline(myReadFile,add)){//read address one at a time and implement the policies
	       len=add.size();
	       add1[0]=add[0];
	       add1[1]=add[1];

	       for(i=2;i<14-len;i++)//adjust the addresses and change them into binary representation
	           add1[i]='0';

	       for(i=14-len;i<10;i++)
	           add1[i]=add[i-12+len];

	           add1[10]='0';
	           add1[11]=add[len-1];

		for(i=2;i<10;i++){//convert to binary representation
		    arr=binary(add1[i]);
		    for(int j=0;j<4;j++){
		    	bin[4*(i-2)+j]=arr[j];
		    }
		}
		      cacheAccess++;

		if(add1[11]=='w')//if given w read write bit is 1
		    rdwr=1;
		else if(add1[11]=='r')//if given r it is zero
		    rdwr=0;


		if(rdwr==0){//using read write bit get the number of read and write accesses
			readAccess++;
		}
		if(rdwr==1){
			writeAccess++;
		}

		temp=Values(bin,cache_size,block_size,associativity);

		if(policy==0)//use the given policy it return s 1 if it is a miss else zero
		    count=Random_policy(temp[1],temp[2],ways,rdwr);
		if(policy==1)
		    count=LRU_policy(temp[1],temp[2],ways,rdwr);
		if(policy==2)
		    count=pseudo_LRU(temp[1],temp[2],ways,rdwr);

		if(rdwr==0)//increase read write bits whether it has a read or write bit
		    readMisses+=count;
		else
		    writeMisses+=count;

	}

	conflictMisses=cacheMisses-compulsaryMisses;//number of conflict misses =total-compulsory
        
        
        int x=0;
	if(policy==0||policy==1){
	    for(int i=0;i<pow(2,p[1]);i++)
	        x+=cache[i].size();
	}
	else{
	    for(int i=0;i<tree.size();i++){
                for(int j=ways-1;j<2*ways-1;j++){
		     if(tree[i][j].tag!=-1)
		         x++;
		}
           }
	}
	
	if(associativity==0)
	    capacityMisses=cacheMisses-x;


	cout<<"Cache Size             : "<<cache_size<<endl;
	cout<<"Block Size             : "<<block_size<<endl;


	if(associativity==0)
    cout<<"Associativity          : Fully Associative Cache"<<endl;
	else if(associativity==1)
    cout<<"Associativity          : Direct Mapped Cache"<<endl;
	else
    cout<<"Associativity          : "<<associativity<<"-Way Set Associative Cache"<<endl;


	if(policy==0)
    cout<<"Replacement Policy     : Random Replacement Policy"<<endl;
    else if(policy==1)
    cout<<"Replacement Policy     : LRU Replacement Policy"<<endl;
    else if(policy==2)
    cout<<"Replacement Policy     : Pseudo LRU Replacement Policy"<<endl;


	cout<<"Cache accesses         : "<<cacheAccess<<endl;
	cout<<"Read accesses          : "<<readAccess<<endl;
	cout<<"Write accesses         : "<<writeAccess<<endl;
	cout<<"Cache misses           : "<<cacheMisses<<endl;
	cout<<"Compulsory misses      : "<<compulsaryMisses<<endl;
	cout<<"Capacity misses        : "<<capacityMisses<<endl;
	cout<<"Conflict misses        : "<<conflictMisses<<endl;
	cout<<"Read misses            : "<<readMisses<<endl;
	cout<<"Write misses           : "<<writeMisses<<endl;
	cout<<"Dirty blocks evicted   : "<<evictedCount<<endl;


	return 0;
}
