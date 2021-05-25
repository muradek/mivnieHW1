#include "AVL.h"
#include "cartype.h"
#include "model.h" 
#include "iostream"
#include "CarDealershipManager.h"
#include "assert.h"

// default constructer
// initializes all trees to empty trees
// bestseller to nullptr
DSW::DSW()
{
    typestree = new AVLTree<CarType>();
    zerostree = new AVLTree<CarType>();
    gradedmodels = new AVLTree<Model>();
    bestsellers = new AVLTree<Model>();
}

// destructor
DSW::~DSW()
{
    // i think this should work
    delete typestree;
    delete zerostree;
    delete gradedmodels;
    delete bestsellers;
}


// adds a cartype to the system. 
// inserting it into the typetree, and zeroestree
StatusType DSW::addCarType(int typeId, int numOfModels)
{
    // check that the input is correct
    if( typeId<=0 || numOfModels<=0 )
        return INVALID_INPUT;
    CarType* to_insert;
    try
    {
        to_insert = new CarType(typeId, numOfModels);
    }
    catch (std::exception& e)
    {
        return ALLOCATION_ERROR;
    }
    if(typestree->findNode(*to_insert))
    {
        delete to_insert;
        return FAILURE;
    }
    try
    {
        (typestree->insert(*to_insert));
    }
    catch(std::exception& e)
    {
        delete to_insert;
        return ALLOCATION_ERROR;
    }
    bestsellers->insert(Model(typeId,0,0,0));
    CarType* zeroes_insert;
    try
    {
        zeroes_insert = new CarType(typeId, numOfModels);
    }
    // do i have to delete all the trees? 
    catch(std::exception& e)
    {
        delete zeroes_insert;
        return ALLOCATION_ERROR;
    }
    // insert must also be wrapped in try catch
    // as there is a dynamic mem allocation here as well
    try
    {
        zerostree->insert(*zeroes_insert);
    }
    catch (std::exception& e)
    {
        // may want to check that we are not deleting something twice
        delete zeroes_insert;
        return ALLOCATION_ERROR;
    }
    return SUCCESS;
}

// removes a car type from the system
StatusType DSW::removeCarType(int typeId)
{
    if(typeId <= 0)
        return INVALID_INPUT;
    CarType to_remove = CarType(typeId , 1);
    if(!zerostree->remove(to_remove))
    {
        // if this is false, the type was not in the tree
        return FAILURE;
    }
    // if we are here then the car was in the tree
    AVLTree<CarType>::Node* node_to_remove = typestree->findNode(to_remove);
    assert(node_to_remove != nullptr);
    // remove from the bestsellers tree
    // in order to do so we will find the relevent information
    // from the typestree node
    int grade = node_to_remove->data.best_seller->data.numSold*10;
    int model = node_to_remove->data.best_seller->data.model;
    // remove from the bestsellers tree
    bestsellers->remove(Model(typeId, model, grade, grade/10));
    // best seller will be updated automatically as highest node 
    // of the tree
    while(node_to_remove->data.models->root != nullptr)
    {
        // remove the model from gradedmodels
        // no need to check whats actually there
        gradedmodels->remove(node_to_remove->data.models->root->data);
        // remove the model from the typestree tree node
        node_to_remove->data.models->remove(node_to_remove->data.models->root->data);
    }
    typestree->remove(to_remove);
    return SUCCESS;
}

// Sell a car of typeid, model id and updates system
// throws relevent exceptions
StatusType DSW::sellCarr(int typeId, int modelId)
{
    // check arguments
    if(typeId <= 0 || modelId <=0)
        return INVALID_INPUT;
    CarType finder = CarType(typeId , 1);
    // check that the type was in the tree
    AVLTree<CarType>::Node* tree_of_models_to_update = typestree->findNode(finder);
    if(!tree_of_models_to_update)
        return FAILURE;
    // check that the model was in the tree
    AVLTree<Model>::Node* model_to_update 
        = tree_of_models_to_update->data.models->findNode(Model(typeId,modelId));
    if(!model_to_update)
        return FAILURE;
    // create an updated model to insert
    int grade = model_to_update->data.grade + 10;
    int sold = model_to_update->data.numSold+1;
    Model model_to_insert = Model(typeId, modelId, grade, sold);
    // remove and then insert the updated model
    tree_of_models_to_update->data.models->remove(Model(typeId,modelId));
    try
    {
        tree_of_models_to_update->data.models->insert(model_to_insert);
    }
    catch (std::exception& e)
    {
        // need to decide what to do if a bad allocation happens
        throw ALLOCATION_ERROR;
    }
    // check if this is now the best seller
    // if its a null pointer it was and remains best seller
    if(tree_of_models_to_update->data.best_seller == nullptr)
    {
        //update the bestsellers tree
        bestsellers->remove(Model(typeId,modelId,sold-1,0));
        bestsellers-> insert(Model(typeId,modelId,sold,0));
        //update the best seller in the tree of models
        tree_of_models_to_update->data.best_seller 
            = tree_of_models_to_update->data.models->findNode(model_to_insert);
    }    
    // if it was not a null pointer than check
    // if the current model has now sold more models
    else
    {
        if(tree_of_models_to_update->data.best_seller->data.numSold < sold)
        {
            // update the bestsellers tree
            int model = tree_of_models_to_update->data.best_seller->data.model;
            int grade = tree_of_models_to_update->data.best_seller->data.numSold;
            bestsellers->remove(Model(typeId,model,grade,0));
            bestsellers-> insert(Model(typeId,modelId,sold,0));
            // update the tree of models best seller
            tree_of_models_to_update->data.best_seller 
            = tree_of_models_to_update->data.models->findNode(model_to_insert);

        }
        // final case - equal
        else if(tree_of_models_to_update->data.best_seller->data.numSold == sold)
        {
            
            if(tree_of_models_to_update->data.best_seller->data.model >= modelId)
            {
                // update the bestsellers tree
                int model = tree_of_models_to_update->data.best_seller->data.model;
                int grade = tree_of_models_to_update->data.best_seller->data.numSold;
                bestsellers->remove(Model(typeId,model,grade,0));
                bestsellers-> insert(Model(typeId,modelId,sold,0));
                // update the tree of models best seller
                tree_of_models_to_update->data.best_seller 
                = tree_of_models_to_update->data.models->findNode(model_to_insert);
            }
        }
           
    } 
    // find and remove from zerostree, checking that its there
    assert(zerostree->findNode(finder));
    zerostree->findNode(finder)->data.models->remove(model_to_insert);
    // remove and reinsert in gradedmodels
    gradedmodels->remove(model_to_insert);
    try
    {
        gradedmodels->insert(model_to_insert);
    }
    catch (std::exception& e)
    {
        throw ALLOCATION_ERROR;
    }
    return SUCCESS;  
}

StatusType DSW::MakeComplaint(int typeID, int modelID, int t)
{
    // checking input (in wraped fuction, check if DS==NULL)
    if (typeID < 0 || modelID <= 0)
    {
        return INVALID_INPUT;
    }
    
    // create a cartype with the typeID
    CarType find_ct = CarType(typeID , 1);

    // finding the type at the typestree
    AVLTree<CarType>::Node* ct_to_complaint = typestree->findNode(find_ct);
    if (ct_to_complaint==nullptr) // if the cartype is not in the typestree
    {
        return FAILURE;
    }

    Model find_m = Model(typeID, modelID);
    // finding the model at the type's models tree
    AVLTree<Model>::Node* m_to_complaint = ct_to_complaint->data.models->findNode(find_m);
    if (m_to_complaint==nullptr) // if the model doesnt exist
    {
        return FAILURE;
    }

    // update model's grade in the typestree
    int original_grade = m_to_complaint->data.grade;
    int complaint_grade = t / 100;
    m_to_complaint->data.grade = m_to_complaint->data.grade - complaint_grade;

    // initialize the model we want to insert to models tree
    Model model_to_add= Model(typeID, modelID, m_to_complaint->data.grade, m_to_complaint->data.numSold);

    // check if the type is in the zerostree
    AVLTree<CarType>::Node* ct_node_zeros= zerostree->findNode(find_ct);
    if (ct_node_zeros != nullptr)
    {
        // check if the model is in the ct_node_zeros
        AVLTree<Model>::Node* m_node_zeros = ct_node_zeros->data.models->findNode(find_m);
        if (m_node_zeros != nullptr)
        {
            // remove the model from the zeros tree
            ct_node_zeros->data.removeModel(modelID);
            // insert te model to the grade tree
            gradedmodels->insert(model_to_add);
            return SUCCESS;
        }
    }

    // if the model isnt in the zeros, it must be in the grademodels
    // find the model in the gradesmodel 
    Model model_to_remove = Model(typeID, modelID, original_grade, m_to_complaint->data.numSold);
    //check if model_to_remove is in models tree
    gradedmodels->remove(model_to_remove);
    gradedmodels->insert(model_to_add);
    return SUCCESS;
}

StatusType DSW::GetBestSellerModelByType(int typeID, int * modelID)
{
    //checking input
    //if typeID==0 && DS is empty, we should throe FAILURE
    if (typeID<0)
    {
        return INVALID_INPUT;
    }

    //if typeID=0, we should return system's best seller
    if (typeID==0)
    {
        *modelID = bestsellers->data.model;
        return SUCCESS;
    }

    //check if typeID is in the system
    CarType ct_find = CarType (typeID, 1);
    AVLTree<CarType>::Node* ct_node = typestree->findNode(ct_find);
    if (ct_node==nullptr)
    {
        //the typeID is not in the system
        return FAILURE;
    }

    //the typeId is in the system
    *modelID = ct_node->data.getBestSeller();
    return SUCCESS;
}

StatusType DSW::GetWorstModels(int numOfModels, int *types, int *models)
{

}