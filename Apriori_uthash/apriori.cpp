//header for uthash tables
#include "uthash.h"

//For mapped file
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <assert.h>
#include <unistd.h>

#include "config.h"

#define MAX_CHAR 5

namespace mmap_file
{
  uint32_t file_size;
  int32_t fd;
  char* mmapped_data;
  uint32_t get_file_size(const char* filename);
}

uint32_t mmap_file::get_file_size(const char* filename) {
    struct stat st;
    stat(filename, &st);
    return st.st_size;
}

using namespace std;

struct apriori_table {
		uint32_t *item_key;
    uint32_t *item_value;
		uint32_t item_count;
    uint32_t conf_lift_number;
    float conf_support;
    float lift_support;
    UT_hash_handle hh;
};

namespace data
{
  struct apriori_table * c_table = NULL;
  struct apriori_table * l_table = NULL;
  uint32_t transactions = 0;
  uint32_t max_items = 0;
}



void c1();
void l1();
void generate_c();
void generate_l();
void output(struct apriori_table * );
void get_document_from_db();
void scan_transactions();
void support_to_file(uint32_t );
void prune();
bool check_compatibility(uint32_t * ,uint32_t *, uint32_t);
void set_count(uint32_t * , uint32_t);
void generate_conf_lift_number();
uint32_t scan_lift_support(uint32_t );
void set_conf_lift_support(uint32_t );
void conf_lift_to_file(uint32_t );
void close_mmap_file();

int main(int argc, char const *argv[])
{
  clock_t start = clock();
  is_config_valid();
  struct apriori_table * temp = NULL;
	HASH_CLEAR(hh, data::c_table);
	HASH_CLEAR(hh, data::l_table);
  HASH_CLEAR(hh,temp);
  get_document_from_db();
	c1();
	//cout<<"c1\n";
	//output(data::c_table);
	l1();
	//cout<<"l1\n";
	//output(data::l_table);
  uint32_t index_of_step=1;
  support_to_file(index_of_step);
  index_of_step++;
	while(true)
	{
		generate_c();
		if(HASH_COUNT(data::c_table)==0)
			break;
		//cout<<"\nC"<<index_of_step<<"\n";
		//output(data::c_table);
		prune();
		if (HASH_COUNT(data::c_table)==0)
			break;
		//cout<<"\nC"<<index_of_step<<" after prune \n";
		//output(data::c_table);
		scan_transactions();
		//cout<<"\nC"<<index_of_step<<"after scaning dataset \n";
		//output(data::c_table);
		generate_l();
		if (HASH_COUNT(data::l_table)==0)
			break;
    support_to_file(index_of_step);
    temp = data::l_table;
		//cout<<"\nL"<<index_of_step<<"\n";
		//output(data::l_table);
		index_of_step++;
	}
  HASH_CLEAR(hh, data::c_table);
  HASH_CLEAR(hh, data::l_table);
  data::l_table = temp;
  if(HASH_COUNT(data::l_table)!=0)
  {
    uint32_t count_of_nodes = HASH_COUNT(data::l_table);
    generate_conf_lift_number();
    scan_transactions();
    set_conf_lift_support(count_of_nodes);
    conf_lift_to_file(index_of_step-1);
  }
  clock_t end=clock();
  double tm=(double)(end-start)/(CLOCKS_PER_SEC);
  printf("%0.8f sec\n",tm);
  cout << "Number of transaction: " << data::transactions << "\n";
  cout << "Maximum number of items in one transaction: " << data::max_items << "\n";
  cout << "Minimum support: " << configurations::minimum_support << "\n";
	return 0;
}

void get_document_from_db()
{
  //Get mmapped_data size
  mmap_file::file_size = mmap_file::get_file_size(configurations::database_name.c_str());
  //Open file
  mmap_file::fd = open(configurations::database_name.c_str(), O_RDONLY, 0);
	assert(mmap_file::fd != -1);
	//Execute mmap
	mmap_file::mmapped_data = (char*)mmap(NULL, mmap_file::file_size, PROT_READ, MAP_PRIVATE | MAP_POPULATE, mmap_file::fd, 0);
	assert(mmap_file::mmapped_data != MAP_FAILED);
}

void c1()
{
 uint32_t mmap_index = 0;
 uint32_t number = 0;
 char * chars = (char*)malloc(MAX_CHAR*sizeof(char));
 uint32_t max_items_temp = 0;

 struct apriori_table *iter = NULL;

 size_t i = 0;
 while (i < mmap_file::file_size) {
   switch (mmap_file::mmapped_data[i]) {
     case '0':
     case '1':
     case '2':
     case '3':
     case '4':
     case '5':
     case '6':
     case '7':
     case '8':
     case '9':
       chars[mmap_index] = mmap_file::mmapped_data[i];
       mmap_index++;
       i++;
       break;
     case ',':
       number = 0;
       for (uint32_t p = 0; p < mmap_index; ++p)
         number = number * 10 + chars[p] - '0';
       max_items_temp++;
       free(chars);
       chars = (char*)malloc(5*sizeof(char));
       HASH_FIND_INT(data::c_table, &number, iter);
       if(iter != NULL)
       {
         (*iter->item_value)++;
       }
       else
       {
         struct apriori_table *new_item = (struct apriori_table *)malloc(sizeof(apriori_table));
         new_item->item_key = (uint32_t *)malloc(sizeof(uint32_t));
         memcpy(new_item->item_key, &number, sizeof(uint32_t));
         new_item->item_value = (uint32_t *)malloc(sizeof(uint32_t));
         (*new_item->item_value) = 1;
         new_item->item_count = 1;
         HASH_ADD_KEYPTR( hh, data::c_table, new_item->item_key, new_item->item_count*sizeof(uint32_t), new_item);
       }
       i++;
       mmap_index = 0;
       break;
     case '\n':
       number = 0;
       for (uint32_t p = 0; p < mmap_index; ++p)
         number = number * 10 + chars[p] - '0';
       max_items_temp++;
       free(chars);
       chars = (char*)malloc(5*sizeof(char));
       HASH_FIND_INT(data::c_table, &number, iter);
       if(iter != NULL)
       {
         (*iter->item_value)++;
       }
       else
       {
         struct apriori_table *new_item = (struct apriori_table *)malloc(sizeof(apriori_table));
         new_item->item_key = (uint32_t *)malloc(sizeof(uint32_t));
         memcpy(new_item->item_key, &number, sizeof(uint32_t));
         new_item->item_value = (uint32_t *)malloc(sizeof(uint32_t));
         (*new_item->item_value) = 1;
         new_item->item_count = 1;
         HASH_ADD_KEYPTR( hh, data::c_table, new_item->item_key, new_item->item_count*sizeof(uint32_t), new_item);
       }
       data::transactions++;
       if(data::max_items<max_items_temp)
         data::max_items = max_items_temp;
       max_items_temp = 0;
       mmap_index = 0;
       i++;
       break;
     default:
       i++;
       break;
   }
 }
}

void output(struct apriori_table * T)
{
	cout<<"\n";
	struct apriori_table *ii = NULL;
	for(ii=T; ii != NULL; ii=(apriori_table*)ii->hh.next) {
		for (uint32_t i = 0; i < ii->item_count; i++)
		{
			printf("%d ", ii->item_key[i]);
		}
		cout<<" ---(frequency(support))----->> "<< *ii->item_value;
		cout<<"\n";
 	}
}

void l1()
{
	struct apriori_table *ii  = NULL;
	for(ii=data::c_table; ii != NULL; ii=(apriori_table*)ii->hh.next) {
		if ((*ii->item_value) >= configurations::minimum_support)
		{
			struct apriori_table *new_item = (struct apriori_table *)malloc(sizeof(apriori_table));
      new_item->item_key = (uint32_t *)malloc(ii->item_count*sizeof(uint32_t));
      memcpy(new_item->item_key, ii->item_key, ii->item_count*sizeof(uint32_t));
			new_item->item_value = (uint32_t *)malloc(sizeof(uint32_t));
			memcpy(new_item->item_value, ii->item_value, sizeof(uint32_t));
			new_item->item_count = ii->item_count;
			HASH_ADD_KEYPTR( hh, data::l_table, new_item->item_key, new_item->item_count*sizeof(uint32_t), new_item);
		}
	}
}

void generate_c()
{
	HASH_CLEAR(hh,data::c_table);
	struct apriori_table *ii = NULL;
	struct apriori_table * jj = NULL;
	for(ii=data::l_table; ii != NULL; ii=(apriori_table*)ii->hh.next) {
		for(jj=ii; jj != NULL; jj=(apriori_table*)jj->hh.next) {
			if(jj==ii)
				continue;
			uint32_t * a = (uint32_t *) malloc(ii->item_count*sizeof(uint32_t));
			uint32_t * b =  jj->item_key;
			for (uint32_t i = 0; i < ii->item_count; i++)
			{
				a[i] = ii->item_key[i];
			}
			if(check_compatibility(a, b, ii->item_count))
			{
				free(a);
				a = (uint32_t *) malloc((ii->item_count+1)*sizeof(uint32_t));
				for (uint32_t i = 0; i < ii->item_count; i++)
				{
					a[i] = ii->item_key[i];
				}
				a[ii->item_count] = b[jj->item_count-1];
				sort(a, a + ii->item_count+1);
				struct apriori_table *new_item = (struct apriori_table *)malloc(sizeof(apriori_table));
        new_item->item_key = (uint32_t *)malloc((ii->item_count+1)*sizeof(uint32_t));
        memcpy(new_item->item_key, a, (ii->item_count+1)*sizeof(uint32_t));
				new_item->item_value = (uint32_t *)malloc(sizeof(uint32_t));
	    	(*new_item->item_value) = 0;
				new_item->item_count = ii->item_count+1;
				HASH_ADD_KEYPTR( hh, data::c_table, new_item->item_key, new_item->item_count*sizeof(uint32_t), new_item);
			}
      free(a);
		}
	}
}

bool check_compatibility(uint32_t *a, uint32_t *b, uint32_t a_size)
{
	bool compatible=true;
	for (size_t i = 0; i < a_size-1; i++) {
		if ((a[i] != b[i]))
		{
			compatible=false;
			break;
		}
	}
	return compatible;
}

void prune()
{
	struct apriori_table *ii = NULL;
	struct apriori_table *jj = NULL;
	uint32_t k;
	for(ii=data::c_table; ii != NULL; ii=(apriori_table*)ii->hh.next) {
	  uint32_t * a = ii->item_key;
	  for(uint32_t i = 0;i<ii->item_count;i++)
	  {
	    uint32_t * b = (uint32_t *) malloc((ii->item_count-1)*sizeof(uint32_t));
	    k = 0;
	    for (uint32_t j = 0; j < ii->item_count; ++j)
	    {
	      if(j==i)
	        continue;
	      b[k] = a[j];
	      k++;
	    }
	    bool founded = false;
	    for(jj=data::l_table; jj != NULL; jj=(apriori_table*)jj->hh.next) {
	      uint32_t b_i = 0, a_j = 0;

	      while (b_i < (ii->item_count-1) && a_j < jj->item_count)
	      {
	          if( jj->item_key[a_j] <b[b_i] )
	              a_j++;
	          else if( jj->item_key[a_j] == b[b_i] )
	          {
	              a_j++;
	              b_i++;
	          }
	          else if( jj->item_key[a_j] > b[b_i] )
	              break;
	      }

	      if(b_i >= (ii->item_count-1))
	      {
	        founded = true;
	        break;
	      }
	    }
	    free(b);
	    if(!founded)
	      {
	        (*ii->item_value) = -1;
	        break;
	      }
	  }
	}
	struct apriori_table *temp = NULL;

	for(ii=data::c_table; ii != NULL; ii=(apriori_table*)ii->hh.next) {
		if ((*ii->item_value) != -1)
		{
			struct apriori_table *new_item = (struct apriori_table *)malloc(sizeof(apriori_table));
      new_item->item_key = (uint32_t *)malloc(ii->item_count*sizeof(uint32_t));
      memcpy(new_item->item_key, ii->item_key, ii->item_count*sizeof(uint32_t));
			new_item->item_value = (uint32_t *)malloc(sizeof(uint32_t));
			memcpy(new_item->item_value, ii->item_value, sizeof(uint32_t)*ii->item_count);
			new_item->item_count = ii->item_count;
			HASH_ADD_KEYPTR( hh, temp, new_item->item_key, new_item->item_count*sizeof(uint32_t), new_item);
		}
	}
	HASH_CLEAR(hh,data::c_table);
	if(HASH_COUNT(temp)>0)
  {
    for(ii=temp; ii != NULL; ii=(apriori_table*)ii->hh.next) {
  			struct apriori_table *new_item = (struct apriori_table *)malloc(sizeof(apriori_table));
        new_item->item_key = (uint32_t *)malloc(ii->item_count*sizeof(uint32_t));
        memcpy(new_item->item_key, ii->item_key, ii->item_count*sizeof(uint32_t));
        new_item->item_value = (uint32_t *)malloc(sizeof(uint32_t));
  			memcpy(new_item->item_value, ii->item_value, sizeof(uint32_t)*ii->item_count);
  			(*new_item->item_value) = (*ii->item_value);
  			new_item->item_count = ii->item_count;
  			HASH_ADD_KEYPTR( hh, data::c_table, new_item->item_key, new_item->item_count*sizeof(uint32_t), new_item);
  	}
  }
  HASH_CLEAR(hh,temp);
}

void scan_transactions()
{
  uint32_t mmap_index = 0;
 	uint32_t number = 0;
 	uint32_t line_index = 0;
  uint32_t * line = (uint32_t*)malloc(data::max_items*sizeof(uint32_t));
  char * chars = (char*)malloc(MAX_CHAR*sizeof(char));
 	uint32_t i = 0;
 	while (i < mmap_file::file_size) {
     switch (mmap_file::mmapped_data[i]) {
       case '0':
       case '1':
       case '2':
       case '3':
       case '4':
       case '5':
       case '6':
       case '7':
       case '8':
       case '9':
         chars[mmap_index] = mmap_file::mmapped_data[i];
         mmap_index++;
         i++;
         break;
       case ',':
         number = 0;
         for (uint32_t p = 0; p < mmap_index; ++p)
           number = number * 10 + chars[p] - '0';
         line[line_index] = number;
         line_index++;
         free(chars);
         chars = (char*)malloc(MAX_CHAR*sizeof(char));
         mmap_index = 0;
         i++;
         break;
       case '\n':
         number = 0;
         for (uint32_t p = 0; p < mmap_index; ++p)
           number = number * 10 + chars[p] - '0';
         line[line_index] = number;
         line_index++;
         if(line_index>0)
         {
           set_count(line, line_index);
         }
         line_index = 0;
         free(line);
         line = (uint32_t*)malloc(data::max_items*sizeof(uint32_t));
         free(chars);
         chars = (char*)malloc(MAX_CHAR*sizeof(char));
         mmap_index = 0;
         i++;
         break;
       default:
         i++;
         break;
     }
 	}
}

void set_count(uint32_t * a, uint32_t a_size)
{
	struct apriori_table *ii = NULL;
	for(ii=data::c_table; ii != NULL; ii=(apriori_table*)ii->hh.next) {
		uint32_t true_count=0;
		if (ii->item_count<=a_size)
		{
			for (uint32_t i = 0; i < ii->item_count; ++i)
			{
				uint32_t * b = ii->item_key;
				for (uint32_t j = 0; j < a_size; ++j)
				{
					if(b[i]==a[j])
					{
						true_count++;
						break;
					}
				}
			}
		}
		if (true_count==ii->item_count)
		{
			(*ii->item_value)++;
		}
	}
}

void generate_l()
{
	HASH_CLEAR(hh,data::l_table);

	struct apriori_table *ii = NULL;

	for(ii=data::c_table; ii != NULL; ii=(apriori_table*)ii->hh.next) {
		if((*ii->item_value) >= configurations::minimum_support)
		{
			struct apriori_table *new_item = (struct apriori_table *)malloc(sizeof(apriori_table));
      new_item->item_key = (uint32_t *)malloc(ii->item_count*sizeof(uint32_t));
      memcpy(new_item->item_key, ii->item_key, ii->item_count*sizeof(uint32_t));
			new_item->item_value = (uint32_t *)malloc(sizeof(uint32_t));
		  memcpy(new_item->item_value, ii->item_value, sizeof(uint32_t));
			new_item->item_count = ii->item_count;
			HASH_ADD_KEYPTR( hh, data::l_table, new_item->item_key, new_item->item_count*sizeof(uint32_t), new_item);
		}
	}
}

void generate_conf_lift_number()
{
	for(struct apriori_table *ii = data::l_table; ii != NULL; ii=(apriori_table*)ii->hh.next) {
	   for(uint32_t k = 0; k < ii->item_count; ++k) {
       uint32_t m = 0;
       uint32_t * denominator = (uint32_t *)malloc((ii->item_count-1)*sizeof(uint32_t));
       for (size_t i = 0; i < (ii->item_count); i++) {
         if(k == i){
           continue;
         }
         denominator[m] = ii->item_key[i];
         m++;
       }
       struct apriori_table *new_item = (struct apriori_table *)malloc(sizeof(apriori_table));
       new_item->item_key = (uint32_t *)malloc((ii->item_count-1)*sizeof(uint32_t));
       memcpy(new_item->item_key, denominator, (ii->item_count-1)*sizeof(uint32_t));
       new_item->conf_lift_number = ii->item_key[k];
       new_item->item_value = (uint32_t *)malloc(sizeof(uint32_t));
       (*new_item->item_value) = 0;
       new_item->item_count = (ii->item_count-1);
       HASH_ADD_KEYPTR(hh, data::c_table, new_item->item_key, (new_item->item_count)*sizeof(uint32_t), new_item);
       free(denominator);
   }
 }
}
void set_conf_lift_support(uint32_t temp_t_count)
{
  struct apriori_table *ii = NULL;
  struct apriori_table *jj = data::l_table;
  uint32_t group_counter = HASH_COUNT(data::c_table)/temp_t_count;
  uint32_t k = 0;
	for(ii = data::c_table; ii != NULL; ii=(apriori_table*)ii->hh.next) {
    if(k==group_counter)
    {
      jj=(apriori_table*)jj->hh.next;
      k = 0;
    }
    ii->conf_support = ((float)(*jj->item_value)/(float)(*ii->item_value));
    uint32_t support = scan_lift_support(ii->conf_lift_number);
    ii->lift_support = ((float)(ii->conf_support*data::transactions)/(float)(support));
    k++;

  }
}

uint32_t scan_lift_support(uint32_t num)
{
      uint32_t support = 0;
      uint32_t mmap_index = 0;
    	uint32_t number = 0;
    	char * chars = (char*)malloc(MAX_CHAR*sizeof(char));
    	size_t i = 0;
    	while (i < mmap_file::file_size) {
        switch (mmap_file::mmapped_data[i]) {
          case '0':
          case '1':
          case '2':
          case '3':
          case '4':
          case '5':
          case '6':
          case '7':
          case '8':
          case '9':
            chars[mmap_index] = mmap_file::mmapped_data[i];
    		    mmap_index++;
    			  i++;
            break;
          case ',':
            number = 0;
            for (uint32_t p = 0; p < mmap_index; ++p)
              number = number * 10 + chars[p] - '0';
            if(number==num)
              support++;
            free(chars);
            chars = (char*)malloc(MAX_CHAR*sizeof(char));
            mmap_index = 0;
            i++;
            break;
          case '\n':
            number = 0;
            for (uint32_t p = 0; p < mmap_index; ++p)
              number = number * 10 + chars[p] - '0';
            if(number==num)
              support++;
            free(chars);
            chars = (char*)malloc(MAX_CHAR*sizeof(char));
            mmap_index = 0;
            i++;
            break;
          default:
            i++;
            break;
        }
    	}
      return support;
}

void support_to_file(uint32_t index)
{
  uint32_t id = 0;
  struct apriori_table *ii = NULL;
  ofstream output_file("output.csv", ofstream::app);
  if (!output_file.is_open())
  {
    cout << "Unable to open file";
    exit(1);
  }
  output_file << "L" << index << "\n";
  output_file << "ID" << "\t";
  for (size_t i = 0; i < index; i++) {
    output_file << "Item" << "\t";
  }
  output_file << "Support" << "\n";
  for(ii = data::l_table; ii != NULL; ii=(apriori_table*)ii->hh.next) {
    output_file << id << "\t";
    id++;
    for (size_t i = 0; i < ii->item_count; i++) {
      output_file << ii->item_key[i] << "\t";
    }
    output_file << *ii->item_value << "\n";
  }
  output_file << "\n";
  output_file.close();
}

void conf_lift_to_file(uint32_t index)
{
  uint32_t id = 0;
  ofstream output_file("output.csv", ofstream::app);
  if (!output_file.is_open())
  {
    cout << "Unable to open file";
    exit(1);
  }
  output_file << "L" << index << "(confidence  and lift)"<< "\n";

  output_file << "ID" << "\t";
  for (size_t i = 0; i < index-1; i++) {
    output_file << "Item" << "\t";
  }
  output_file << "To" << "\t";
    output_file << "Confidence" << "\t";
    output_file << "Lift" << "\t";
  output_file << "\n";
  for(struct apriori_table * ii = data::c_table; ii != NULL; ii=(apriori_table*)ii->hh.next) {
    if (ii->conf_support >= configurations::minimum_confidence) {
      output_file << id << "\t";
      id++;
      for (size_t i = 0; i < ii->item_count; i++) {
        output_file << ii->item_key[i] << "\t";
      }
      output_file << ii->conf_lift_number << "\t";

        output_file << ii->conf_support << "\t";
        output_file << ii->lift_support << "\t";
      output_file << "\n";
    }
  }
  output_file << "\n";
  output_file.close();
}

void close_mmap_file()
{
  int32_t rc = munmap(mmap_file::mmapped_data, mmap_file::file_size);
  assert(rc == 0);
  close(mmap_file::fd);
}
