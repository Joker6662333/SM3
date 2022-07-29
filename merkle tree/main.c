#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif


#include <stdio.h>
#include "string.h"
#include "mbedtls/md.h"
typedef unsigned int uint;
unsigned char* _mbedtls_shax_test(mbedtls_md_type_t md_type,  char* m)
{
	int len, i;
	int ret;
	//const char *message = "Hello";
	char* message = &m;
	unsigned char digest[32];

	mbedtls_md_context_t ctx;
	const mbedtls_md_info_t* info;

	printf("message is:%s\r\n", message);

	/* 1. init mbedtls_md_context_t structure */
	mbedtls_md_init(&ctx);

	/* 2. get md info structure pointer */
	info = mbedtls_md_info_from_type(md_type);

	/* 3. setup md info structure */
	ret = mbedtls_md_setup(&ctx, info, 0);
	if (ret != 0) {
		goto exit;
	}

	/* 4. start */
	ret = mbedtls_md_starts(&ctx);
	if (ret != 0) {
		goto exit;
	}

	/* 5. update */
	ret = mbedtls_md_update(&ctx, (unsigned char*)message, strlen(message));
	if (ret != 0) {
		goto exit;
	}

	/* 6. finish */
	ret = mbedtls_md_finish(&ctx, digest);
	if (ret != 0) {
		goto exit;
	}

	/* show */
	printf("%s digest context is:[", mbedtls_md_get_name(info));
	len = mbedtls_md_get_size(info);
	for (i = 0; i < len; i++) {
		printf("%02x", digest[i]);
	}
	printf("]\r\n");

exit:
	/* 7. free */
	mbedtls_md_free(&ctx);

	return digest;
}

// Merkle Tree �ṹ�嶨�� 
typedef struct MerkleTreeNode {
	struct MerkleTreeNode* left;
	struct MerkleTreeNode* right;
	struct MerkleTreeNode* parent;
	unsigned char hash_num;		// ��ϣֵ ����������������У���ֵΪ��ǰ�ڵ������� 
	char* data;
}MerkleTree;

#define New_Merkle_Node(mt, tree_depth) {	\
	mt = (MerkleTree *)malloc(sizeof(MerkleTree)); \
	mt->left = NULL; \
	mt->right = NULL; \
	mt->parent = NULL; \
	mt->hash_num = (uint)tree_depth; \
	mt->data = NULL;	\
	}


// ��ӡ Merkle tree 
int first = 0;
void Print_Merkle_Tree(MerkleTree* mt, int high)
{
	MerkleTree* p = mt;
	int i;

	if (p == NULL) {
		return;
	}
	if (p->left == NULL && p->right == NULL) {
		printf("\n");
		for (i = 0; i < high - p->hash_num; i++)
			printf("      ");

		printf("--->%2d\n", p->data);
		first = 1;

		return;
	}
	else {
		Print_Merkle_Tree(mt->left, high);

		if (first == 1) {
			for (i = 0; i < high - p->hash_num; i++)
				printf("      ");

			printf("--->");
		}
		else
			printf("--->");

		printf("%2d", p->hash_num);
		first = 0;

		Print_Merkle_Tree(mt->right, high);
		//printf("\n");
	}
}

// ����һ���ַ�����hashֵ 
uint hash_string(char* key) {
	uint cal = 11, hash = 0;
	while (*key != '\0' && *key != 0) {
		hash = hash * cal + *key;
		key++;
	}
	return hash & 0x7FFFFFFF;
}

// ����һ���ַ�����hashֵ 
unsigned char* hash1(unsigned char* m) {
	unsigned char* result = _mbedtls_shax_test(MBEDTLS_MD_SHA256, m);
	return result;
}

// ��������������hash ֵ 
unsigned char* hash2(char* m1,char* m2) {
	char* tmp = NULL;
	sprintf(tmp, "%s%s", m1, m2);
	
	unsigned char* result= _mbedtls_shax_test(MBEDTLS_MD_SHA256, tmp);
	return result;
}

// ����������������ҵ��յ�Ҷ�ӽڵ㷵��Ҷ�ӽ��ָ��
// ���û���ҵ�����Ϊ�����������򷵻� NULL 
// ����� 
// �ҵ����һ���ڵ� 
MerkleTree* Find_Last_Node(MerkleTree* mt) {
	MerkleTree* p = mt, * tmp;

	if (p->left == NULL && p->right == NULL)	// Ҷ�ӽڵ� 
		return p;
	else if (p->right == NULL && p->left != NULL)
		return Find_Last_Node(p->left);
	else if (p->right != NULL)
		return Find_Last_Node(p->right);
}

// �������һ���ڵ㣬�ҵ������λ�� 
MerkleTree* Find_Empty_Node(MerkleTree* mt) {
	MerkleTree* p = mt->parent;

	while (p->left != NULL && p->right != NULL && p->parent != NULL) {
		p = p->parent;
	}
	if (p->parent == NULL && p->left != NULL && p->right != NULL) {		// ������
		//printf("��ǰ�ڵ�λ�ã�p->hash_num=%d, �����ˣ����� \n", p->hash_num); 
		return NULL;
	}
	else {
		//printf("��ǰ�ڵ�λ�ã�p->hash_num=%d \n", p->hash_num); 
		return p;
	}
}

// �������Ĺ�ϣֵ 
void update_hash_Merkle(MerkleTree* mt, int tree_depth)
{
	if (mt == NULL)
		return;
	if (mt->hash_num == 0) {
		mt->hash_num = 0;
	}
	else if (mt->hash_num == 1) {
		mt->hash_num = hash2(hash1((unsigned char*)mt->left->data), hash1((unsigned char*)mt->right->data));
	}
	else {
		update_hash_Merkle(mt->left, tree_depth - 1);
		update_hash_Merkle(mt->right, tree_depth - 1);
		mt->hash_num = hash2(mt->left->hash_num, mt->right->hash_num);
	}
}

// Merkle tree ��ʼ�� (�ݹ�ʵ��)
MerkleTree* Creat_Merkle_Tree(MerkleTree* mt, int* arr, int nums, int tree_depth)
{
	MerkleTree* node, * tmp, * p;
	int i;
	if (nums == 0) {
		//update_hash_Merkle(mt, tree_depth);
		printf("�������\n");

		if (mt != NULL) {
			first = 0;
			printf("\n��ʼ��ӡ��ǰ Merkle ��:\n");
			Print_Merkle_Tree(mt, mt->hash_num);
			printf("\n");
		}
		return mt;
	}
	else {
		printf("Ҷ�ӽڵ� [%d] arr=%d, nums=%d, tree_depth=%d\n", __LINE__, *arr, nums, tree_depth);
		// ÿ�����һ��Ҷ�ӽڵ㣬�����������������
		// ����һ������� 
		New_Merkle_Node(node, 0);
		node->data = *arr;

		// ��� mt Ϊ�գ�˵����ǰû����	
		if (mt == NULL) {
			// ����ͷ���
			New_Merkle_Node(mt, 1);
			mt->left = node; 	// ��ͷ�ڵ㸳ֵ 
			node->parent = mt;
			// ��ǰ���߶� +1 
			tree_depth++;

			// �ݹ�
			printf("��ͷ��� [%d] tree_depth=%d, mt->hash_num=%d\n", __LINE__, tree_depth, mt->hash_num);
			mt = Creat_Merkle_Tree(mt, arr + 1, nums - 1, tree_depth);
		}
		// ��� mt ��Ϊ��,mtΪͷ��� 
		else
		{
			p = Find_Empty_Node(Find_Last_Node(mt));	// ������ǰ�����ҵ�һ���յ�Ҷ�ӽڵ㣬��������ʱ����NULL 

			// ���flag Ϊ1 ˵�����ڿյ� ��Ҷ�ӽڵ� 
			if (p != NULL) {
				// �������¾���Ҷ�ӽڵ㣬��ֱ�Ӹ�ֵ 
				if (p->left->hash_num == 0 && p->right == NULL)
				{
					p->right = node;
					node->parent = p;
				}
				else
				{
					i = p->hash_num - 1;
					// ����һ���µ�ͷ���
					New_Merkle_Node(tmp, i);
					p->right = tmp;
					tmp->parent = p;

					p = p->right;
					printf("������ [%d] tree_depth=%d, hash_num=%d\n", __LINE__, tree_depth, p->hash_num);

					i--;
					// ����������ȴ���ͬ����ȵ����� 
					while (i > 0) {
						// �������
						New_Merkle_Node(tmp, i);
						p->left = tmp;
						tmp->parent = p;

						p = p->left;
						i--;
					}

					// Ҷ�ӽڵ㸳ֵ 
					p->left = node;
					node->parent = p;
				}
				mt = Creat_Merkle_Tree(mt, arr + 1, nums - 1, tree_depth);
			}
			//���û�пյ�Ҷ�ӽڵ㣬���½�һ��ͷ��� 
			else
			{
				tmp = mt;	// ���浱ǰͷ���
				tree_depth++; 		// ���߶� +1 

				// ����һ���µ�ͷ���
				New_Merkle_Node(mt, tree_depth);
				mt->left = tmp; 	// ��ͷ�ڵ㸳ֵ 
				tmp->parent = mt;

				// ����ͷ��� -  Ҷ�ӽڵ� ֮������нڵ� 
				i = tree_depth - 1;	// �ڶ���ڵ� 

				// ͷ��� right ��ֵ  
				New_Merkle_Node(tmp, i);
				mt->right = tmp;
				tmp->parent = mt;

				i--;
				p = mt->right;
				printf("������ [%d] tree_depth=%d, hash_num=%d\n", __LINE__, tree_depth, p->hash_num);

				// ����������ȴ���ͬ����ȵ����� 
				while (i > 0) {
					// �������
					New_Merkle_Node(tmp, i);
					p->left = tmp;
					tmp->parent = p;

					p = p->left;
					i--;
				}
				// Ҷ�ӽڵ㸳ֵ 
				p->left = node;
				node->parent = p;

				// �ݹ���� 
				mt = Creat_Merkle_Tree(mt, arr + 1, nums - 1, tree_depth);
			}
		}
	}
}


int main()
{
	// , This Is Cielle.
	char* array[] = { '1','2','3','4'};
	MerkleTree* mt = NULL;

	Creat_Merkle_Tree(mt, array, 4, 0);


	return 0;
}


