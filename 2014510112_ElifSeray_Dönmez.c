/*
 * 
 * Elif Seray Dönmez
 * 2014510112
 *
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define dim 2 // dimension
#define REC_SIZE sizeof(struct City)


FILE *output;
FILE *indexFile;
int pageSize, bfr;
int totalNodes = 0;

typedef struct Node
{
    double coord[dim]; // Storing coordinates
    int leftPageNumber; // Storing the page which includes the record
    int rightPageNumber;
    struct Node *left, *right;
}Node;

typedef struct City
{
	int plateNumber;
	char name[32];
	double area;
	int population;
	int populationDensity;
	int populationOfCityCenter;
	char region[32];
	double latitude;
	double longitude;
}City;

typedef struct List {
    double coord[dim];
    int leftPageNumber;
    int rightPageNumber;
    struct List * next;
} List;

Node* root;
List* head = NULL;

void printCity(City);
Node *insertNode(List*, Node*, double[], int, int, int);
void sequentialReading(char*);
void readFile(char*);
void indexNodeCoordinate(List*, double[]);
int *searchNode(Node*, double[], int);
int *pageSearch(Node*, double[], int);
void createData(char*);
void createIndex(List*);
void readIndex(char*);
int cityFind(char*, int*);
void printMenu(int*);
void insertNewRecord(char*);


int main()
{	
    root = NULL;
	output = fopen("output.dat", "r+");

	if(!output) // if there is no output file
	{
		printf("Please enter page size -> ");
		scanf("%d", &pageSize);
		sequentialReading("data.dat");
		bfr = pageSize / REC_SIZE; // number of records in a page
        createData("data.dat");
        int *t;
        double coord[] = {501105,298852};
        t = pageSearch(root, coord, 0); // t[0] -> page, t[1] -> depth
        createIndex(head);
    }
    else // if we have an output file
    {
        printf("Please enter page size -> ");
        readIndex("index.dat"); 
        bfr = pageSize / REC_SIZE;
        readFile("output.dat");
        fclose(indexFile);
    }

    int answer;
    ab:

    ////////////  Menu  ///////////
    printMenu(&answer);

    while (answer > 4 || answer < 1)
	{
		printf("\nEnter a valid choice by pressing ENTER key again");
		printMenu(&answer);
	}

	switch (answer)
	{
		case 1: insertNewRecord("data.dat");
			goto ab;
			break;
		case 2: 
			if(root == NULL)
			{
				printf("Root is null!\n");
				goto ab;
				break;
			}
				
			char name[32];
			double coord[2];
			int *page;
			printf("Name :");
			scanf("%s", name);
			printf("Latitude :");
			scanf("%lf", &coord[0]);
			printf("Longitude :");
			scanf("%lf", &coord[1]);
			page = pageSearch(root, coord, 0);
			cityFind(name, page);
			goto ab;
			break;
		case 3: printf("Program is terminating \n");
			break;
	}

	return 0;
}

void sequentialReading(char *fileName)
{
    FILE *fp = fopen(fileName, "rb");
    City city;

    if(!fp) 
    {
        printf("Could not open file!");
        return;
    }
    fread(&city, REC_SIZE, 1, fp);
    
    while(!feof(fp)) 
    {
    	printCity(city);
        fread(&city, REC_SIZE, 1, fp);
    }
    fclose(fp);
}

void readFile(char *fileName)
{
	FILE *fp = fopen(fileName, "rb");
    City city;

    if(!fp) 
    {
        printf("Could not open file!");
        return;
    }
    fread(&city, REC_SIZE, 1, fp);

    int count = 1;
    while(!feof(fp)) 
    {
        fread(&city, REC_SIZE, 1, fp);
        count++;

        if((count % bfr) == 0) // if the current record is the last record in the page
        {
        	// passing the remaining part of the page
            int a = pageSize - (bfr * REC_SIZE); 
            fseek(fp, a, SEEK_CUR);
        }
    }
    fclose(fp);
}

// saving the coordinates of the index nodes
void indexNodeCoordinate(List *root, double data[]) 
{
    List *list = root;
    while (list->next != NULL) 
        list = list->next;

    // adding a new variable
    list->next = malloc(sizeof(List));
    list->next->coord[0] = data[0]; // 0 -> x
    list->next->coord[1] = data[1]; // 1 -> y
    list->next->leftPageNumber = data[2]; // 2 -> left page number
    list->next->rightPageNumber = data[3]; // 3 -> right page number
    list->next->next = NULL;
}

Node *insertNode(List *head, Node *root, double coord[], int depth, int leftPage, int rightPage)
{   
    if (root == NULL)
    {
    	Node* newNode = malloc(sizeof(Node));
 		int i;
	    for (i=0; i < dim; i++)
	       newNode->coord[i] = coord[i];
	    
	 	newNode->leftPageNumber = leftPage;
	 	newNode->rightPageNumber = rightPage;
	    newNode->left = NULL;
	    newNode->right = NULL;

        double data[4];
        data[0] = coord[0];
        data[1] = coord[1];
        data[2] = leftPage;
        data[3] = rightPage;
        indexNodeCoordinate(head, data); // after inserting a node, inserts coordinates

	    return newNode; // Stores the page includes this record.
    }
 
    int split = depth % dim; // Current dimension of comparison - for splitting
 
 	// Compare new nodes with root and decşde where to go on the tree - coordinates
    if (coord[split] < (root->coord[split])) // Left
    {
    	if(root->left == NULL)
        	root->leftPageNumber = -1;
        
        root->left = insertNode(head, root->left, coord, depth + 1, leftPage, rightPage);
    	
    }

    else // Right
    {
    	if(root->right == NULL)
        	root->rightPageNumber = -1;

        root->right = insertNode(head, root->right, coord, depth + 1, leftPage, rightPage);
    }
 
    return root;
}

int *searchNode(Node* root, double coord[], int depth)
{
    static int temp[3];

    if (root == NULL)
    	return 0;

    if (root->coord[0] == coord[0] && root->coord[1] == coord[1])
    { 
        temp[0] = root->leftPageNumber;
        temp[1] = root->rightPageNumber;
        temp[2] = depth;
    	return temp;
    }
 
    int split = depth % dim; // current dimension - splitting
 
    // Compare current point with root 
    if (coord[split] < root->coord[split])
        return searchNode(root->left, coord, depth + 1);

    else
        return searchNode(root->right, coord, depth + 1); 
}

int *pageSearch(Node* root, double coord[], int depth)
{
    static int temp[2];

    if (root == NULL){
        temp[0] = 0; // 0 -> Page
        temp[1] = 0; // 1 -> Depth
        return temp;
    }

    int split = depth % dim; // Current dimension - split
 
    // Compare the point with root
    if (coord[split] >= root->coord[split]) // Right
    { 
        if(root->right == NULL)
        {
            temp[0] = root->rightPageNumber;
            temp[1] = depth;
            return temp;
        }
        return pageSearch(root->right, coord, depth + 1);
    }

    else // Left
    {
    	if(root->left == NULL)
        {
            temp[0] = root->leftPageNumber;
            temp[1] = depth;
            return temp;
        }
        return pageSearch(root->left, coord, depth + 1);        
    }
}

void createData(char *fileName)
{
	int page = 0;
	int finalPage = 0;
	int depth = 0;
	int isFull = 0;
	double temp[2];
	int split = 0;
	int median = 0;
	City city;
	int *pageDepthArray;
	int i;
	City *initialCity;  // initialized - null city
    int isFirstPage = 0;

    head = malloc(sizeof(List));
	
	FILE *isExist = fopen("output.dat", "r+"); // Output file is exist?
    if (!isExist)
    {
        output = fopen("output.dat", "w+");	// If not, open an empty file
        fclose(output);
        output = fopen("output.dat", "r+b"); // Create one
    }
   
    FILE *fp = fopen(fileName, "rb"); // Read input file
    if(!fp) 
    {
        printf("Could not open file!");
        return;
    }
	
	if(root == NULL)
	{
		page = 0;
		finalPage = 0;
		depth = 0;
		initialCity = calloc(1, pageSize);
		fseek(output, page*pageSize, SEEK_SET);
		fwrite(initialCity, pageSize, 1, output);
		free(initialCity);
        isFirstPage = 1;
	}

	fseek(fp, 0, SEEK_SET);
	fread(&city, REC_SIZE, 1, fp);

	while(!feof(fp)) 
	{
		temp[0] = city.latitude;
        temp[1] = city.longitude;

        pageDepthArray = pageSearch(root, temp, 0);
        page = pageDepthArray[0];
        depth = pageDepthArray[1];

        // Choose the correct page, take the correct city
        fseek(output, page*pageSize, SEEK_SET); 
        City *cities = calloc(1, pageSize); //cities in the page
        fread(cities, pageSize, 1, output); 
        isFull = 0;

        for (i = 0; i < bfr; i++)
        {
        	if(cities[i].plateNumber == 0)
        	{
				cities[i] = city;
				break;
			}
			else{
				isFull++;
			}
        }
        fseek(output, page*pageSize, SEEK_SET);
        fwrite(cities, pageSize, 1, output);
        free(cities);

        if(isFull == bfr) // Split
        { 
        	finalPage++;
        	initialCity = calloc(1, pageSize);
			fseek(output, finalPage*pageSize, SEEK_SET); // Creates new page
			fwrite(initialCity, pageSize, 1, output);
			free(initialCity);

			fseek(output, page*pageSize, SEEK_SET);
			split = (depth + 1) % dim; 

			if(isFirstPage == 1)
			{
				split = 0;
                isFirstPage = 0;
			}

			City cityArray[bfr];
			median = bfr / 2;
	        City *cities = calloc(1, pageSize); //Split ederken kullanılacak
	        fread(cities, pageSize, 1, output);

	        ////////////  Bubble Sort  ///////////

	        for (i = 0; i < bfr; i++)
	        {
				cityArray[i] = cities[i];
	        }

	        int j = 0;
	        City tempCity;
	        for(i = 0; i < bfr; i++)
			{
			    for(j = 0; j < bfr-1; j++)
			    {
			    	if(split == 0)
			    	{
				        if(cityArray[j].latitude > cityArray[j+1].latitude)
				        {
				            tempCity = cityArray[j];
				            cityArray[j] = cityArray[j+1];
				            cityArray[j+1] = tempCity;

				        }
				    }

				    else
				    {
				    	if(cityArray[j].longitude > cityArray[j+1].longitude)
				        {
				            tempCity = cityArray[j];
				            cityArray[j] = cityArray[j+1];
				            cityArray[j+1] = tempCity;
				        }
				    }
			    }
			}

			temp[0] = cityArray[median].latitude;
            temp[1] = cityArray[median].longitude;

	        root = insertNode(head, root, temp, 0, page, finalPage);
            totalNodes++;

	        fseek(output, finalPage*pageSize, SEEK_SET); // Move
	        cities = calloc(1, pageSize); 
       		fread(cities, pageSize, 1, output);

			for (i = median; i < bfr; i++) // cities to be moved to a new page
			{
				cities[i] = cityArray[i];
			}

			fseek(output, finalPage*pageSize, SEEK_SET);
        	fwrite(cities, pageSize, 1, output);
       		free(cities);

       		initialCity = calloc(1, pageSize);
			fseek(output, page*pageSize, SEEK_SET);
			fwrite(initialCity, pageSize, 1, output);
			free(initialCity);

			fseek(output, page*pageSize, SEEK_SET);
			cities = calloc(1, pageSize); 
       		fread(cities, pageSize, 1, output);

			for (i = 0; i < median; i++)
				cities[i] = cityArray[i];

	        fseek(output, page*pageSize, SEEK_SET);
        	fwrite(cities, pageSize, 1, output);
       		free(cities);

       		temp[0] = city.latitude;
        	temp[1] = city.longitude;

       		pageDepthArray = pageSearch(root, temp, 0); // search the page if it is exist
        	page = pageDepthArray[0];

        	fseek(output, page*pageSize, SEEK_SET);
        	cities = calloc(1, pageSize); 
       		fread(cities, pageSize, 1, output);

       		for (i = 0; i < bfr; i++)
	        {
	        	if(cities[i].plateNumber == 0)
	        	{
					cities[i] = city;
					break;
				}
	        }
	        fseek(output, page*pageSize, SEEK_SET);
        	fwrite(cities, pageSize, 1, output);
       		free(cities);
        }
        fread(&city, REC_SIZE, 1, fp); // read the next city - splitted    
	}
	fclose(output);
}

void createIndex(List *root)
{
    FILE *isExist = fopen("index.dat", "r+"); // Is the index file exist

    if (!isExist)
    {
        indexFile = fopen("index.dat", "w+"); // If not, opens an empty file
        fclose(indexFile);
        indexFile = fopen("index.dat", "r+b"); // Creates the index file
    }

    double data[4];
    List * current = root;
    fseek(indexFile, 0, SEEK_SET); 
    fwrite(&pageSize, sizeof(int), 1, indexFile);

    while (current != NULL) 
    {
        data[0] = current->coord[0];
        data[1] = current->coord[1];
        data[2] = current->leftPageNumber;
        data[3] = current->rightPageNumber;
        fwrite(&data, 4 * sizeof(double), 1, indexFile);
        current = current->next;
    }
    
    fclose(indexFile);
}

void readIndex(char *filename)
{
    FILE *fp = fopen(filename, "rb");
    int page;
    double data[4];

    if(!fp) 
    {
        printf("Could not open file");
        return;
    }

    fread(&page, sizeof(int), 1, fp);
    pageSize = page;
    fread(&data, 4*sizeof(double), 1, fp);

    while(!feof(fp)) 
    {
        fread(&data, 4*sizeof(double), 1, fp);
    }
    fclose(fp);
}

void printCity(City city)
{
	if(city.plateNumber == 0)
	{
		printf("------\n");
	}
	else
	{
		printf("Name : %s\n", city.name);
		printf("Latitude - x : %f\n", city.latitude);
		printf("Longitude - y : %f\n", city.longitude);
		printf("Plate Number : %d\n", city.plateNumber);
		printf("Region : %s\n", city.region);
		printf("Population : %d\n", city.population);
		printf("Area : %f\n", city.area);
		printf("Population Density : %d\n", city.populationDensity);
		printf("Population of City Center : %d\n", city.populationOfCityCenter);
	}
}

void printMenu(int* answer)
{
	printf("You can perform the following tasks: \n");
	printf("(1) Insert a new record \n");
	printf("(2) Search for a record by coordinates and name \n");
	printf("(3) Quit \n");
	printf("Please Select one... \n");
	scanf("%d", answer);
}

int cityFind(char name[] , int page[]) // Finding an existing city
{
	int pageNum = page[0];
	FILE *fp = fopen("output.dat", "r+");
	fseek(fp, pageNum*pageSize, SEEK_SET);

	City *city = calloc(1, pageSize);
	fread(city, pageSize, 1, output);

	int i;
	for(i = 0; i < bfr; i++)
	{
		if(strcmp(city[i].name, name) == 0)
		{
			printCity(city[i]);
			return 1; // True
		}
	}	

	printf("City could not found!\n");
	return 0; // False
}

void insertNewRecord(char *fileName)
{
	FILE *fp = fopen(fileName, "r+b");
	City city;
	int *page;
	double coord[2];

	printf("Enter Name -> "); scanf("%s", city.name);
	printf("\nEnter Longitude -> "); scanf("%lf", &city.longitude);
	printf("\nEnter Latitude -> "); scanf("%lf", &city.latitude);
	printf("\nEnter Plate Number -> "); scanf("%d", &city.plateNumber);
	printf("\nEnter Population -> "); scanf("%d", &city.population);
	printf("\nEnter Region -> "); scanf("%s", city.region);
	printf("\nEnter Area -> "); scanf("%lf", &city.area);
	printf("\nEnter Population Density -> "); scanf("%d", &city.populationDensity);
	printf("\nEnter Population of the City Center -> "); scanf("%d", &city.populationOfCityCenter);
	
	coord[0] = city.latitude;
	coord[1] = city.longitude;

	page = pageSearch(root, coord, 0);
	int c = cityFind(city.name, page); // If city is found, returns 1

	if(c == 0) // City is not exist, createa new city
	{
		fseek(fp, 0, SEEK_END);
		fwrite(&city, sizeof(City), 1, fp);
		fclose(fp);
		sequentialReading("data.dat");
	}
	else
		printf("The record is already exist in the file!\n");
}