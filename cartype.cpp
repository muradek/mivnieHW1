#include "cartype.h"
#include "iostream"

CarType::CarType(int type, int numOfModels):type(type)
{
    //initializing a sorted array for insert into AVLTree
    // had to use new as size is determined during runtime
    Model* modelsarr= new Model[numOfModels];
    for(int i=0; i<numOfModels; i++)
    {
        modelsarr[i].setModel(this->type, i);
    }
    
    //initializing the models AVLTree
    //AVLTree<Model>* modelstree = new AVLTree<Model>();
    this->models = new AVLTree<Model>::AVLTree();
    //turning the sorted array into AVLTree
    models = AVLTree<Model>::arrToAVLTree(modelsarr, 0, numOfModels-1); 

    // the model with the highest value at initialization
    // is defined to be the model with the lowest model number
    // which is defined as the best selling model at initialization
    delete[] modelsarr;
}

//destructor for CarType, must delete models
CarType::~CarType()
{
    //delete models;
}
// returns pointer to most sold model
AVLTree<Model>::Node* CarType::getMostSold()
{
    return models->getHighest();
}
//comparing operator, compares types by typeID
bool CarType::operator<(const CarType& cartype)
{
    return this->type > cartype.type;
}

//comparing operator, compares types by typeID
bool CarType::operator>(const CarType& cartype)
{
    return this->type < cartype.type;
}

//comparing operator, compares types by typeID
bool CarType::operator==(const CarType& cartype)
{
    return this->type == cartype.type;
}

bool CarType::addModel(int model_num, int grade, int numSold)
{
    Model model_to_add= Model(this->type, model_num, grade, numSold);
    return(this->models->insert(model_to_add));
}

bool CarType::removeModel(int model_num)
{
    Model model_to_delete= Model(this->type, model_num);
    return(this->models->remove(model_to_delete));
}