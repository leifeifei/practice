/*
File name:Btree
Author:雷飞飞
Date:2016/8/4 ~ 2016/8/9
Description:B树
B树的性质：如果有一棵M阶的B树，那么有
          1.树中每个结点含有且最多X个孩子，即满足ceil(M/2) <= X <= M;
          2.除根结点和叶结点外，其他每个结点至少有ceil(M/2)个孩子。
          3.除根结点之外的结点的关键字个事N必须满足ceil(M/2) -1 <= N <= M-1;
          4.根结点至少有两个孩子。

程序思路：
         1.基于链表的模板类B树。
         2.默认每个结点的孩子指针都是指向其孩子所在链表的头结点.
         3.删除或者插入以后，若违反性质，则调用相关函数，使其满足性质
         4.插入时，边插入边排序。
           1.先判断该值是否已存在于B树中。
           2.不存在，找到比其大的关键字的结点，将其插入其后。
           3.若不满足性质。即调用Find_mid函数，使其满足性质.
         5.删除时。
           1.先判断该结点是否存在
           2.判断该结点是否有无左右孩子。
           3.若无左右孩子，判断结点所在位置，然后根据实际位置相关情况进行相关操作后
             再删除。若删除后所在链表不满足性质，调用Fix_up，使其满足性质。
           4.若有左右孩子，找到其右子树里面最小结点值，与其替换，再将，替换结点删除，
             若删除后，替换结点所处链表不满足性质，则调用Fix_up函数，使其满足性质
        6.输出时，是一层一层输出的，即先将根结点的左子树的最小关键字所处链表输出，然后
          向上递归输出，类似于树的后序遍历。
        7.结合树和链表的特点，所以我设置结点里面，有两个父结点指针。
          两个指针指向如下图
                  parent    parent_n
                      \       / 
                       [结点]
*/

#include <iostream>
#include <math.h>
using namespace std;

//申明链表类
template<class T>
class List_B;

//申明B树
template<class T>
class Btree;

//定义节点类
template <class T>
class Node
{
friend class List_B<T>;
friend class Btree<T>;
public:
    Node(T _valve)
    {
        valve = _valve;
        next = NULL;
        pre_next = NULL;
        left_child = NULL;
        right_child= NULL;
        parent = NULL;
        parent_n = NULL;
        ptr = NULL;
    }
private:
    T valve;
    Node<T> * next;         //指向下一个结点
    Node<T> * pre_next;     //指向前一个结点
    Node<T> * parent;       //指向父节点
    Node<T> * parent_n;     //指向另一父结点
    Node<T> * left_child;   //指向左孩子
    Node<T> * right_child;  //指向右孩子
    List_B<T> * ptr;        //方便后续操作，定义一个链表指针
};

//定义链表类
template<class T>
class List_B
{
friend class Btree<T>;
public:
    List_B()
    {
       head = NULL;
       curr = NULL;
    }
    ~List_B()
    {
        if (head != NULL)
        {
            delete head;
            head = NULL;
        }
        if(curr != NULL)
        {
            delete curr;
            curr = NULL;
        }
    }

    //在链表后插入数据
    Node<T> * Push_Back(Node<T> * NewNode)
    {
        if (head == NULL)
        {
            head = NewNode;
            curr = NewNode;
        }
        else
        {
            curr = head;
            while (curr->next != NULL)
            {
                curr = curr->next;
            }
            curr->next = NewNode;
            NewNode->pre_next = curr;
        }
        Sort_List(head,NULL);
        return head;
    }
    
    //在链表头插入数据
    Node<T> * Push_Front(Node<T> * NewNode)
    {
        head->pre_next = NewNode;
        NewNode->next = head;
        NewNode->parent = head->parent;
        NewNode->parent_n = head->parent_n;
        if (head->parent != NULL)
        {
            if (head->parent->left_child == head)
            {
                head->parent->left_child = NewNode;
            }
            else
            {
                head->parent->right_child = NewNode;
            }
        }
        if (head->parent_n != head->parent)
        {
            if (head->parent_n->left_child == head)
            {
                head->parent_n->left_child = NewNode;
            }
            else
            {
                head->parent_n->right_child = NewNode;
            }
        }
        head->ptr = NULL;
        NewNode->ptr = new List_B<T>;
        NewNode = NewNode->ptr->Push_Back(NewNode);
        return NewNode;
    }

    //删除头结点
    Node<T> * Pop_Front(Node<T> * Delete_Node)
    {
         Node<T> * tmp = Delete_Node->next;
         tmp->parent = Delete_Node->parent;
         tmp->parent_n = Delete_Node->parent_n;
         if (Delete_Node == Delete_Node->parent->left_child)
         {
            Delete_Node->parent->left_child = tmp;
         }
         else
         {
            Delete_Node->parent->right_child = tmp;
         }
         if (tmp->parent != tmp->parent_n)
         {
            if (Delete_Node == Delete_Node->parent_n->left_child)
            {
                Delete_Node->parent_n->left_child = tmp;
            }
            else
            {
                Delete_Node->parent_n->right_child = tmp;
            }
         }
         tmp->pre_next = NULL;
         tmp->ptr = new List_B<T>;
         tmp->ptr->Push_Back(tmp);
         Delete_Node->next = NULL;
         delete Delete_Node;
         Delete_Node = NULL;
         return tmp;
    }

    //合并两个链表
    void Merge_List(List_B * from)
    {
        Node<T> * tmp = head;
        while(tmp->next != NULL)
        {
            tmp = tmp->next;
        }
        tmp->next = from->head;
        from->head->pre_next = tmp;
    }

    //交换链表里面两个结点的值
    void Swap_Node(Node<T> * left,Node<T> * right)
    {
        if (left == right)
        {
            return;
        }
        Node<T> * before = left->pre_next;

        //若交换的两个结点相邻，且left前面有结点时
        if (left->pre_next != NULL && left->next == right) 
        {
            right->pre_next = before;
            before->next = right;
            left->pre_next = right;
            if (right->next == NULL)
            {
                left->next = NULL;
            }
            else
            {
                left->next = right->next;
                left->next->pre_next = left;
            }
            right->next = left;
            return;
        }

        //若替换的两个结点 相邻，且left的前面无结点时
        if (left->pre_next == NULL && left->next == right)
        {
            right->pre_next = NULL;
            left->pre_next = right;
            if (right->next == NULL)
            {
                left->next = NULL;
            }
            else
            {
                left->next = right->next;
                left->next->pre_next = left;
            }
            right->next = left;
            left->ptr = NULL;
            right->ptr = NULL;
            right->ptr = new List_B<T>;
            right->ptr->head = right;
            right->parent = left->parent;
            right->parent_n = left->parent_n;
            left->parent = NULL;
            left->parent_n = NULL;
            return;
        }

        Node<T> * after = left->next;
        if (before != NULL)    //当left前面有结点时
        {
            left->next = right->next;
            left->pre_next = right->pre_next;
            right->next = after;
            after->pre_next = right;
            right->pre_next->next = left;
            right->pre_next = before;
            before->next = right;
        }
        else
        {
            left->next = right->next;
            left->pre_next = right->pre_next;
            right->next = after;
            after->pre_next = right;
            right->pre_next->next = left;
            right->pre_next = NULL;
            left->ptr = NULL;
            right->ptr = NULL;
            right->ptr = new List_B<T>;
            right->ptr->head = right;
            right->parent = left->parent;
            right->parent_n = left->parent_n;
            left->parent = NULL;
            left->parent_n = NULL;
        }
           
    }

    //利用快排的思想，每次把链表与一个基准值分成两部分，进行排序
    void Sort_List(Node<T> * left,Node<T> * right )
    {
        if (left == NULL)
        {
            return;
        }
        if( left->next == NULL)
        {
            return;
        }
        if (left == right)
        {
            return;
        }
        Node<T> * base = left;        //以base->valve作为基准值
        Node<T> * tmp = NULL;
        Node<T> * fast_pre = NULL;
        Node<T> * slow = left;        //当比基准值小时，移动该指针。
        Node<T> * fast = left->next;  //当比基准值小时，和slow对应的值交换。
        while (fast != right)
        {
            if (fast->valve < base->valve)
            {
                tmp = slow;
                slow = slow->next;
                Swap_Node(slow,fast);
                slow = tmp->next;
            }
            fast = fast->next;
        }
        Swap_Node(base,slow);        //slow所在结点的值始终比基准值小，所以要交换
        Sort_List(slow,base);        //将比基准值小的数据排序
        Sort_List(base->next,NULL);  //比基准值大的数据进行排序
    }
    
    int Size_List()
    {
        Node<T> * tmp = head;
        int num = 0;
        while(tmp != NULL)
        {
            num++;
            tmp = tmp->next;
        }
        return num;
    }

    void Output_List()
    {
        Node<T> * tmp = head;
        if (tmp == NULL)
        {
            return;
        }
        cout << "[";
        while (tmp != NULL)
        {
            cout << tmp->valve << '\t';
            tmp = tmp->next;
        }
        cout << "]";
    }
private:
    Node<T> * head;
    Node<T> * curr;
};

//定义B树类
template<class T>
class Btree
{
public:
    Btree(int _M)
    {
        root = NULL;
        M = _M;
        key = M - 1;
    }

    ~Btree()
    {
        if (root != NULL)
        {
            delete root;
            root = NULL;
        }
    }

    //插入数据
    bool Insert_Btree(T data)
    {
        //首先查找当前树中有无data值，有，返回data所在的结点，没有，返回比data大的前一个结点
        Node<T> * tmp = Find_Btree(data); 
        
        if ( root == NULL && tmp == NULL)
        {
            root = new Node<T>(data);
            root->ptr = new List_B<T>; 
            root = root->ptr->Push_Back(root);
            return true;
        }
        if ( tmp->valve == data)
        {
            cout << "该值已存在" << endl;
            return false;
        }
        Node<T> * NewNode = new Node<T>(data);

        //当根结点为空
        if (root->left_child == NULL)
        {
            int count = root->ptr->Size_List();
            if (count < key )   //满足B树性质时
            {
                root->ptr->Push_Back(NewNode);
            }
            if (count == key ) //不满足B树性质时
            {
                root->ptr->Push_Back(NewNode);
                root = Find_Mid(root);
            }
        }
        else    //当根结点不为空时
        {
            if (tmp->pre_next == NULL)
            {
                if (tmp == tmp->parent->right_child && data < tmp->valve)
                {
                    tmp = tmp->ptr->Push_Front(NewNode);
                }
                else
                {
                    tmp = tmp->ptr->Push_Back(NewNode); 
                }
            }
            else
            {
                tmp = Find_Head_List(tmp);      //找到所插入结点链表的头结点
                tmp = tmp->ptr->Push_Back(NewNode); 
            }
            if (tmp->ptr->Size_List() >= M) //若该链表结点数(也即关键字树)大于M
            {
                Find_Mid(tmp);              //不满足性质，调用此函数分割链表
            }
        }
        return true;
    }
     
    //删除数据
    bool Delete_BtreeNode(T data)
    {
        //首先查找当前树中有无data值，有，返回data所在的结点，没有，返回比data大的前一个结点
        Node<T> * node = Find_Btree(data); 
        if ( node->valve != data)
        {
            return false;
        }

        Node<T> * p = node->parent;    //node的父结点
        Node<T> * p_n = node->parent_n;
        Node<T> * gp = NULL;           //node的祖父结点
        Node<T> * tmp = NULL;
        
        //当其左右孩子为空时
        if (node->left_child == NULL && node->right_child == NULL)
        {
            //当其结点无父结点，前面也无结点，但有下一个结点时。
            if (p == NULL && node->pre_next == NULL && node->next != NULL)
            { 
               tmp = node->next;
               tmp->pre_next = NULL;
               tmp->ptr = new List_B<T>;
               tmp->ptr->head = tmp;
               root = tmp;
            }

            //当其结点无父结点，前有结点，后有结点时
            if (p == NULL && node->pre_next != NULL && node->next != NULL)
            {
                tmp = node->pre_next;
                tmp->next = node->next;
                node->next->pre_next = tmp;
            }

            //当其结点无父结点，前有结点,后无结点时
            if (p == NULL && node->pre_next != NULL && node->next == NULL)
            {        
                tmp = node->pre_next;      
                tmp->next = NULL;
                tmp = Find_Head_List(tmp);
                if (tmp->ptr->Size_List() <= ceil(M/2) - 1 && tmp->parent != NULL)
                {
                    Fix_Up(tmp);
                }
            }

            //当其结点有父结点，后无结点时。
            if (p != NULL && node->next == NULL)
            {
                gp = p->parent;
                if (node == p->left_child)
                {
                    p->ptr->Merge_List(p->right_child->ptr);
                    p->right_child->parent = NULL;
                    p->right_child->parent_n = NULL;
                    p->right_child = NULL;
                    p->left_child = NULL;
                    if (p->ptr->Size_List() >= key )
                    {
                        tmp = Find_Mid(p);
                    }
                }
                else
                {
                    tmp = p->left_child;
                    tmp->ptr->Merge_List(p->ptr);
                    p->right_child = NULL;
                    p->left_child = NULL;
                    p->ptr = NULL;
                    tmp->parent = gp;
                    tmp->parent_n = p->parent_n;
                    p->parent = NULL;
                    p->parent_n = NULL;
                    if ( tmp->ptr->Size_List() >= key )
                    {
                        tmp = Find_Mid(tmp);
                    }
                    
                }
                
                //当其祖父结点为空时，且父结点所在链表只有一个关键字时
                if(gp == NULL)
                {
                    root = tmp;
                }
            }
            
            //当其结点有父结点,后有结点时，说明该结点处于链表头
            if (p != NULL && node->next != NULL)
            {
                tmp = node->ptr->Pop_Front(node); 

                //当其父结点所在链表结点数(关键字数) 小于最少关键字数时，即不满足性质时
                if(tmp->ptr->Size_List() <= ceil(M/2) - 1)
                {
                    tmp = Fix_Up(tmp);
                }
                return true;
            }
            delete node;
            node = NULL;
        }
        else //当其结点有左右孩子时，找到其右孩子的最小结点，即最小关键字，与之替换关键字，再删除
        {
            tmp = Find_Min(node->right_child);
            T del_valve = node->valve;
            node->valve = tmp->valve;
            tmp->valve = del_valve;

            tmp = tmp->ptr->Pop_Front(tmp);
            if(tmp->ptr->Size_List() <= ceil(M/2) - 1)  //不满足性质时
            {
                tmp = Fix_Up(tmp);
            }             
        } 
        return true; 
    }

    //该函数主要用于在删除结点时，B树不满足性质时，调用该函数使其满足性质，
    //在删除结点时，找寻的时其右边子树的最小结点与其替换，所以调用此函数时，
    //其结点tmp无左右孩子的,
    Node<T> * Fix_Up(Node<T> * tmp)
    {
        Node<T> * p = tmp->parent;   //其父结点
        Node<T> * p_n = tmp->parent_n;
        Node<T> * gp = p->parent;    //祖父结点
        Node<T> * q = NULL;
        
        //当结点是父结点的左孩子，若父结点的右孩子关键字数大于最少关键字数时，
        //将其父结点插入其左孩子所在链表，将其右孩子所在链表头结点关键字搬至父结点处，
        //在删除右孩子的头结点,返回其父结点的右孩子
        if (tmp == p->left_child && p->right_child->ptr->Size_List() > ceil(M/2))
        {
            q = new Node<T>(p->valve);    
            tmp->ptr->Push_Back(q);
            p->valve = p->right_child->valve;
            tmp = p->right_child->ptr->Pop_Front(p->right_child);
            return tmp;   
        }

        //当结点是其父结点的右孩子时，若父结点的左孩子关键字数大于最少关键字数时，
        //将其父结点插入其右孩子的链表头，将其左孩子的最大关键字移至父结点，在删除左孩子的
        //最大关键字的结点，返回其父结点的左孩子
        if (tmp == p->right_child && p->left_child->ptr->Size_List() > ceil(M/2) )
        {
            Node<T> * t = p->left_child;
            while (t->next != NULL)
            {
                t = t->next;
            }
            q = new Node<T>(p->valve);
            tmp->ptr->Push_Front(q);
            p->valve = t->valve;
            t->pre_next->next = NULL;
            t->pre_next = NULL;
            delete t;
            t = NULL;
            return tmp; 
        }

        //当其父结点的下一结点不为空时，若父结点的下一结点右孩子的关键字数大于最少关键字数时，
        //及该结点是其父结点的右孩子时，将父结点的下一结点的关键字移至其右孩子所在链表后，
        //将父结点下一结点右孩子关键字移至其头结点处,返回其父结点的下一结点的右孩子
        if (p->next != NULL && p->next->right_child->ptr->Size_List() > ceil(M/2) && tmp == p->right_child  )
        {
            q = new Node<T>(p->next->valve);
            tmp->ptr->Push_Back(q);
            p->next->valve = p->next->right_child->valve;
            tmp = p->next->right_child->ptr->Pop_Front(p->next->right_child);
            return tmp; 
        }
        
        //当其父结点的前一结点不为空时，若其父结点的前一结点的左孩子数大于最少关键字数时，
        //将其父结点的前一结点的关键字移至移至其左孩子的前面，再将父结点的前一结点的左孩子的
        //最大关键字移至其父结点处，返回的是其父结点的前一结点的左孩子
        if (p->pre_next != NULL && p->pre_next->left_child->ptr->Size_List() > ceil(M/2) && tmp == p->left_child)
        {
            Node<T> * t = p->pre_next->left_child;
            while (t->next != NULL)
            {
                t = t->next;
            }
            q = new Node<T>(p->valve);
            tmp->ptr->Push_Front(q);
            p->pre_next->valve = t->valve;
            t->pre_next->next = NULL;
            t->pre_next = NULL;
            delete t;
            t = NULL;
            return tmp; 
        }

        //当其父结点前后无结点时，先将其父结点的左孩子与父结点合并，再将左右孩子合并
        //返回合并后的头结点 
        if (p->pre_next == NULL && p->next == NULL)
        {


            p->ptr = NULL;

            //1.合并左右孩子
            p->left_child->ptr->Push_Back(p);
            p->left_child->ptr->Merge_List(p->right_child->ptr);
            tmp = p->left_child;
            tmp->parent = gp;

            //2.若其祖父结点不为空时
            if (gp != NULL)
            {
                if (p == gp->left_child)
                {
                    gp->left_child = tmp;
                }
                else
                {
                    gp->right_child = tmp;
                }
            }

            //3.若其父结点的左孩子还有孩子时
            if (p->left_child->left_child != NULL)
            {
                p->left_child = p->pre_next->right_child;
                p->left_child->parent_n = p;

                p->right_child = p->next->left_child;
                p->right_child->parent = p;
            }
            else //若其父结点的左孩子没有孩子时
            {
                p->left_child = NULL;
                p->right_child->parent = NULL;
                p->right_child = NULL;
            }
            
            p->parent = NULL;
            p->parent_n = NULL;

            //若4.p是根结点时
            if (p == root)
            {
                root->ptr = NULL;
                root = tmp;
            }
            return tmp;
        }

        //若父结点的前一结点为空，后一结点不为空时，返回合并后的头结点
        if (p->pre_next == NULL && p->next != NULL)
        {
            //1.先分离父结点和下一结点
            tmp = p->next;
            tmp->pre_next = NULL;
            tmp->parent = gp;
            tmp->parent_n = p->parent_n;
            p->next = NULL;
            p->ptr = NULL;
            tmp->ptr = new List_B<T>;
            tmp->ptr->head = tmp;

            //2.若祖父结点不为空时
            if (gp != NULL)
            {
                if (p == gp->left_child)
                {
                    gp->left_child = tmp;
                }
                else
                {
                    gp->right_child = tmp;
                }
            }
            
            //3.合并父结点的左右孩子
            p->left_child->ptr->Push_Back(p);
            p->left_child->ptr->Merge_List(p->right_child->ptr);
            tmp->left_child = p->left_child;
            p->left_child->parent = tmp;
            tmp = p->left_child;
            
            //4.若父结点的左孩子还有孩子时
            if (p->left_child->right_child != NULL)
            {
                p->left_child = p->pre_next->right_child;
                p->left_child->parent_n = p;

                p->right_child = p->next->left_child;
                p->right_child->parent = p;
            }
            else
            {
                p->left_child = NULL;
                p->right_child->parent = NULL;
                p->right_child = NULL;
            }

            p->parent = NULL;
            p->parent_n = NULL;

            //5.若合并后的父结点关键字数不满足性质时
            if (tmp->parent->ptr->Size_List() <= ceil(M/2) - 1)
            {
                tmp = Fix_Up(tmp->parent);
            }
            return tmp;
        }

        //若父结点的前一结点不为空，后一结点为空时，返回合并后的头结点
        if (p->pre_next != NULL && p->next == NULL)
        {
            //1.分离父结点与其前一结点
            tmp = p->pre_next;
            p->left_child->parent = tmp;
            tmp->next = NULL;
            p->pre_next = NULL;

            //2.合并父结点的左右孩子
            p->left_child->ptr->Push_Back(p);
            p->left_child->ptr->Merge_List(p->right_child->ptr);
            tmp->right_child = p->left_child;
            p->left_child->parent = tmp;
            tmp = p->left_child;
            
            //3.若父结点的左孩子还有孩子时
            if (p->left_child->left_child != NULL)
            {
                p->left_child = p->pre_next->right_child;
                p->left_child->parent_n = p;

                p->right_child = p->next->left_child;
                p->right_child->parent = p;
            }
            else
            {
                p->left_child = NULL;
                p->right_child->parent = NULL;
                p->right_child = NULL;
            }
            p->parent = NULL;
            p->parent_n = NULL;

            //4.若合并后的父结点关键字数不满足性质时
            if (tmp->parent->ptr->Size_List() <= ceil(M/2) - 1)
            {
                tmp = Fix_Up(tmp->parent);
            }
            return tmp;
        }

        //若若父结点的前后有结点，返回合并后的头结点
        if (p->pre_next != NULL && p->next != NULL)
        {
            //1.将父结点与其前后结点分离
            tmp = p->next;
            p->left_child->parent = p->pre_next;
            tmp->left_child = p->left_child;
            p->pre_next->next = tmp;
            tmp->pre_next = p->pre_next;
            p->pre_next->right_child = p->left_child;
            tmp->left_child = p->left_child;
            p->pre_next = NULL;
            p->next = NULL;

            //2.合并其左右孩子
            p->left_child->ptr->Push_Back(p);
            p->left_child->ptr->Merge_List(p->right_child->ptr);
            p->left_child->parent = tmp;
            tmp = p->left_child;
            
            //3.若父结点的左孩子还有孩子时
            if (p->left_child->left_child != NULL)
            {
               p->left_child = p->pre_next->right_child;
                p->left_child->parent_n = p;

                p->right_child = p->next->left_child;
                p->right_child->parent = p;
            }
            else
            {
                p->left_child = NULL;
                p->right_child->parent = NULL;
                p->right_child = NULL;
            }
            p->parent = NULL;
            p->parent_n = NULL;
            
            //4.若合并后的父结点关键字数不满足性质时
            if (tmp->parent->ptr->Size_List() <= ceil(M/2) - 1)
            {
                tmp = Fix_Up(tmp->parent);
            }
            return tmp;
        }
    }
    
    //在满足B树性质的前提下，分割链表，及找到分割结点，将其返回
    Node<T> * Find_Mid(Node<T> * mid)
    {
        //1.判断mid是否为空
        if (NULL == mid)
        {
            return mid;
        }

        //2.做好分离前的准备
        Node<T> * p = mid->parent;
        Node<T> * gp = NULL;
        Node<T> * frist = mid;
        Node<T> * second = NULL;
        Node<T> * tmp = NULL;

        //3.找到分离点
        int pos = (int)ceil(M/2);
        while (pos--)
        {
            tmp = mid;      //tmp指向分离点的前一结点
            mid = mid->next;
        }

        if (mid->left_child != NULL)
        {
            mid->left_child->parent_n = tmp;
            mid->next->left_child->parent = mid->next;
           
        }
        tmp->next = NULL;
        
        //4.分离
        second = mid->next;
        second->parent = mid;
        frist->parent_n = mid;
        second->pre_next = NULL;

        mid->next = NULL;
        mid->left_child = frist;
        mid->right_child = second;
        mid->pre_next = NULL;
       
        second->ptr = NULL;
        second->ptr = new List_B<T>;
        second->ptr->head = second;

        //5.当分割链表父结点为空时
        if (p == NULL)
        {
            mid->ptr = new List_B<T>;
            mid->ptr->head = mid;
            frist->parent = mid;
            second->parent_n = mid;
            root = mid;
        }

        //6.当分割链表父点不为空时
        else 
        {
            gp = p->parent;
            tmp = Find_Head_List(p);

            //当分离结点比其父结点所在链表头结点值小时
            if (mid->valve < tmp->valve) 
            {
                frist->parent = mid;
                tmp->left_child = second;
                second->parent_n = tmp;
                tmp = tmp->ptr->Push_Front(mid);
            }
            else
            {
                tmp = tmp->ptr->Push_Back(mid);
                if (mid->next != NULL)
                {
                    mid->next->left_child = second;
                    second->parent_n = mid->next;
                }
                if (mid->next == NULL)
                {
                    second->parent_n = mid;
                }
            }

            //分离后，mid所在链表不满足性质时
            if (tmp->ptr->Size_List() >= M)
            {
                frist->parent = mid;
                tmp = Find_Mid(tmp);              
            }

            if (gp == NULL)  //祖父结点为空时
            {
                if (tmp != root)
                {
                    root->ptr = NULL;
                }
                root = tmp;
            }
            tmp->parent = gp;
            mid = tmp;
        }                 
        return mid;
    }
    
    //寻找链表头
    Node<T> * Find_Head_List(Node<T> * tmp)
    {
        if (NULL == tmp)
        {
            return tmp;
        }
        if (tmp->pre_next == NULL)
        {
            return tmp;
        }
        while (tmp->pre_next != NULL)
        {
            tmp = tmp->pre_next;
        }
        return tmp;
    }
   
    //查找B树是否存在data.返回 false 不存在，反之存在。
    Node<T> * Find_Btree(T data)
    {
        if (root == NULL)
        {
            return NULL;
        }
        Node<T> * tmp = root;
        if (data < tmp->valve )   //小于根结点时，在其右子树寻找
        {
            tmp = Find_Left(tmp,data);
            return tmp;
        }
        if (data == tmp->valve)    //等于时将其放回
        {
             return tmp;
        }
        if (tmp->next == NULL)     //根结点下一点为空时，直接在右子树中寻找
        {
            tmp = Find_Right(tmp,data);
            return tmp;
        }
        else 
        {
            tmp = Find_List(tmp,data); //不为空时，调用该函数来寻找
            return tmp;
        }
        return tmp;
    }

    //在B树的左孩子里面寻找data,返回 false 不存在，反之存在。
    Node<T> * Find_Left(Node<T> * tmp,T data)
    {  
        if (tmp == NULL)
        {
            return NULL;
        }
        if (NULL == tmp->left_child)
        {
            return tmp;
        }
       
        Node<T> * Is_Exists = tmp;
        if (tmp->left_child != NULL)
        {
            tmp = tmp->left_child;
            if (data < tmp->valve)
            {
               Is_Exists = Find_Left(tmp,data);
               return Is_Exists;
            }
            if (data == tmp->valve)
            {
                return tmp;
            }
            if (tmp->next == NULL)
            {
                Is_Exists = Find_Right(tmp,data);
                return Is_Exists;
            }
            if (tmp->next != NULL)
            {
                Is_Exists = Find_List(tmp,data);
                return Is_Exists;
            }
        }
        return Is_Exists;
    }

    //在B树右孩子里面寻找data,返回 false 不存在，反之存在.
    Node<T> * Find_Right(Node<T> * tmp,T data)
    {
        if (tmp == NULL)
        {
            return NULL;
        }
        if (tmp->right_child == NULL)
        {
           return tmp;
        }

        tmp = tmp->right_child;
        Node<T> * Is_Exists = tmp;
        if (data < tmp->valve)
        {
            Is_Exists = Find_Left(tmp,data);
            return Is_Exists;
        }
        if (data == tmp->valve)
        {
            return tmp;
        }
        if (tmp->next == NULL)
        {
            Is_Exists = Find_Right(tmp,data);
            return Is_Exists;
        }
        if (tmp->next != NULL)
        {
            Is_Exists = Find_List(tmp,data);
            return Is_Exists;
        }
        return Is_Exists;
    }
    
    //在B树关键字链表寻找data,返回 false 不存在，反之存在。
    Node<T> * Find_List(Node<T> * tmp,T data)
    {
        if (NULL == tmp)
        {
            return NULL;
        }
        if (tmp->next == NULL)
        {
            return tmp;
        }

        tmp = tmp->next;
        Node<T> * Is_Exists = tmp;
        if (data < tmp->valve)
        {
            Is_Exists = Find_Left(tmp,data);
            return Is_Exists;
        }
        if (data == tmp->valve)
        {
             return tmp;
        }
         //当下一结点为空和左孩子为空时
        if (tmp->next == NULL && tmp->right_child == NULL)
        {
            return Is_Exists;
        }

        //当下一结点为空，左孩子不为空时。
        if (tmp->next == NULL && tmp->right_child != NULL)
        {
            Is_Exists = Find_Right(tmp,data);
            return Is_Exists;
        }
         
        if (tmp->next != NULL)
        {
            Is_Exists = Find_List(tmp,data);
            return Is_Exists;
        }
        return Is_Exists;
    }

    Node<T> * Find_Min(Node<T> * tmp)
    {
        if (tmp == NULL)
        {
            return NULL;
        }
        while (tmp->left_child != NULL)
        {
           tmp = tmp->left_child;
        }
        return tmp;
    }

    void Output_Btree()
    {
        //1.先找到最小关键字所在链表
        Node<T> * tmp = Find_Min(root);
        if (tmp == NULL)
        {
            return;
        }
        //2.先将其输出
        tmp = Find_Head_List(tmp);
        tmp->ptr->Output_List();

        //3.再输出其他关键字
        Output_Btree(tmp->parent);  
        cout << endl;
    }

    void Output_Btree(Node<T> * bt)
    {
        if (bt == NULL)
        {
            return;
        }
       
        Node<T> * tmp = Find_Right(bt,bt->valve);
        if (tmp == NULL)
        {
            return;
        }
        tmp = Find_Head_List(tmp);
        tmp->ptr->Output_List();
        Node<T> * p = tmp->parent;
        Node<T> * p_n = tmp->parent_n;
        Node<T> * gp = p->parent;
        
        //如果没有父结点时
        if (p == NULL)
        {
            return;
        }
        if (gp != NULL && p == gp->right_child)
        {
            p->right_child->ptr->Output_List();
        }

        //当结点前为空，后为空时
        if (p->pre_next == NULL && p->next == NULL)
        {
            p->ptr->Output_List();
            Output_Btree(p->parent);
        }

        //当结点后不为空时
        if ( p->next != NULL)
        {
            Output_Btree(p->next);
        }

        //当结点前后不为空时
        if (p->pre_next != NULL && p->next == NULL)
        {
            p = Find_Head_List(p);
            p->ptr->Output_List();
            cout << endl;
            if (p->parent == root && p == p->parent->right_child)
            {
                p->parent->ptr->Output_List();
                return;
            }          
            Output_Btree(p->parent);
        }
    }

private:
    Node<T> * root;
    int M;
    int key;
};

int main(int argc, char const *argv[])
{
    Btree<char> b1(5);
    b1.Insert_Btree('A');
    b1.Insert_Btree('C');
    b1.Insert_Btree('G');
    b1.Insert_Btree('N');
    b1.Insert_Btree('H');
    b1.Insert_Btree('E');
    b1.Insert_Btree('K');
    b1.Insert_Btree('Q');
    b1.Insert_Btree('M');
    b1.Insert_Btree('F');
    b1.Insert_Btree('W');
    b1.Insert_Btree('L');
    b1.Insert_Btree('T');
    b1.Insert_Btree('Z');
    b1.Insert_Btree('D');
    b1.Insert_Btree('P');
    b1.Insert_Btree('R');
    b1.Insert_Btree('X');
    b1.Insert_Btree('Y');
    b1.Insert_Btree('S');
    b1.Output_Btree();
    b1.Delete_BtreeNode('H');
    b1.Delete_BtreeNode('T');
    b1.Delete_BtreeNode('R');
    b1.Delete_BtreeNode('E');
    cout << " after delete" << endl;
    b1.Output_Btree();
    cout << " b1.Insert_Btree('E');" << endl;
    b1.Insert_Btree('E');
    b1.Output_Btree();
    cout << " b1.Insert_Btree('R');" << endl;
    b1.Insert_Btree('R');
    b1.Output_Btree();
    cout << " b1.Insert_Btree('T');" << endl;
    b1.Insert_Btree('T');
    b1.Output_Btree();
    cout << " b1.Insert_Btree('H');" << endl;
    b1.Insert_Btree('H');
    b1.Output_Btree();
    
    return 0;
}
