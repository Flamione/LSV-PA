/**CFile****************************************************************

  FileName    [cecSolve.c]

  SystemName  [ABC: Logic synthesis and verification system.]

  PackageName [Combinatinoal equivalence checking.]

  Synopsis    [Performs one round of SAT solving.]

  Author      [Alan Mishchenko]
  
  Affiliation [UC Berkeley]

  Date        [Ver. 1.0. Started - June 20, 2005.]

  Revision    [$Id: cecSolve.c,v 1.00 2005/06/20 00:00:00 alanmi Exp $]

***********************************************************************/

#include "cecInt.h"

////////////////////////////////////////////////////////////////////////
///                        DECLARATIONS                              ///
////////////////////////////////////////////////////////////////////////

static inline int  Cec_ObjSatNum( Cec_ManSat_t * p, Gia_Obj_t * pObj )             { return p->pSatVars[Gia_ObjId(p->pAig,pObj)]; }
static inline void Cec_ObjSetSatNum( Cec_ManSat_t * p, Gia_Obj_t * pObj, int Num ) { p->pSatVars[Gia_ObjId(p->pAig,pObj)] = Num;  }

////////////////////////////////////////////////////////////////////////
///                     FUNCTION DEFINITIONS                         ///
////////////////////////////////////////////////////////////////////////

/**Function*************************************************************

  Synopsis    [Returns value of the SAT variable.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int Cec_ObjSatVarValue( Cec_ManSat_t * p, Gia_Obj_t * pObj )             
{ 
    return sat_solver_var_value( p->pSat, Cec_ObjSatNum(p, pObj) );
}

/**Function*************************************************************

  Synopsis    [Addes clauses to the solver.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Cec_AddClausesMux( Cec_ManSat_t * p, Gia_Obj_t * pNode )
{
    Gia_Obj_t * pNodeI, * pNodeT, * pNodeE;
    int pLits[4], RetValue, VarF, VarI, VarT, VarE, fCompT, fCompE;

    assert( !Gia_IsComplement( pNode ) );
    assert( Gia_ObjIsMuxType( pNode ) );
    // get nodes (I = if, T = then, E = else)
    pNodeI = Gia_ObjRecognizeMux( pNode, &pNodeT, &pNodeE );
    // get the variable numbers
    VarF = Cec_ObjSatNum(p,pNode);
    VarI = Cec_ObjSatNum(p,pNodeI);
    VarT = Cec_ObjSatNum(p,Gia_Regular(pNodeT));
    VarE = Cec_ObjSatNum(p,Gia_Regular(pNodeE));
    // get the complementation flags
    fCompT = Gia_IsComplement(pNodeT);
    fCompE = Gia_IsComplement(pNodeE);

    // f = ITE(i, t, e)

    // i' + t' + f
    // i' + t  + f'
    // i  + e' + f
    // i  + e  + f'

    // create four clauses
    pLits[0] = toLitCond(VarI, 1);
    pLits[1] = toLitCond(VarT, 1^fCompT);
    pLits[2] = toLitCond(VarF, 0);
    if ( p->pPars->fPolarFlip )
    {
        if ( pNodeI->fPhase )               pLits[0] = lit_neg( pLits[0] );
        if ( Gia_Regular(pNodeT)->fPhase )  pLits[1] = lit_neg( pLits[1] );
        if ( pNode->fPhase )                pLits[2] = lit_neg( pLits[2] );
    }
    RetValue = sat_solver_addclause( p->pSat, pLits, pLits + 3 );
    assert( RetValue );
    pLits[0] = toLitCond(VarI, 1);
    pLits[1] = toLitCond(VarT, 0^fCompT);
    pLits[2] = toLitCond(VarF, 1);
    if ( p->pPars->fPolarFlip )
    {
        if ( pNodeI->fPhase )               pLits[0] = lit_neg( pLits[0] );
        if ( Gia_Regular(pNodeT)->fPhase )  pLits[1] = lit_neg( pLits[1] );
        if ( pNode->fPhase )                pLits[2] = lit_neg( pLits[2] );
    }
    RetValue = sat_solver_addclause( p->pSat, pLits, pLits + 3 );
    assert( RetValue );
    pLits[0] = toLitCond(VarI, 0);
    pLits[1] = toLitCond(VarE, 1^fCompE);
    pLits[2] = toLitCond(VarF, 0);
    if ( p->pPars->fPolarFlip )
    {
        if ( pNodeI->fPhase )               pLits[0] = lit_neg( pLits[0] );
        if ( Gia_Regular(pNodeE)->fPhase )  pLits[1] = lit_neg( pLits[1] );
        if ( pNode->fPhase )                pLits[2] = lit_neg( pLits[2] );
    }
    RetValue = sat_solver_addclause( p->pSat, pLits, pLits + 3 );
    assert( RetValue );
    pLits[0] = toLitCond(VarI, 0);
    pLits[1] = toLitCond(VarE, 0^fCompE);
    pLits[2] = toLitCond(VarF, 1);
    if ( p->pPars->fPolarFlip )
    {
        if ( pNodeI->fPhase )               pLits[0] = lit_neg( pLits[0] );
        if ( Gia_Regular(pNodeE)->fPhase )  pLits[1] = lit_neg( pLits[1] );
        if ( pNode->fPhase )                pLits[2] = lit_neg( pLits[2] );
    }
    RetValue = sat_solver_addclause( p->pSat, pLits, pLits + 3 );
    assert( RetValue );

    // two additional clauses
    // t' & e' -> f'
    // t  & e  -> f 

    // t  + e   + f'
    // t' + e'  + f 

    if ( VarT == VarE )
    {
//        assert( fCompT == !fCompE );
        return;
    }

    pLits[0] = toLitCond(VarT, 0^fCompT);
    pLits[1] = toLitCond(VarE, 0^fCompE);
    pLits[2] = toLitCond(VarF, 1);
    if ( p->pPars->fPolarFlip )
    {
        if ( Gia_Regular(pNodeT)->fPhase )  pLits[0] = lit_neg( pLits[0] );
        if ( Gia_Regular(pNodeE)->fPhase )  pLits[1] = lit_neg( pLits[1] );
        if ( pNode->fPhase )                pLits[2] = lit_neg( pLits[2] );
    }
    RetValue = sat_solver_addclause( p->pSat, pLits, pLits + 3 );
    assert( RetValue );
    pLits[0] = toLitCond(VarT, 1^fCompT);
    pLits[1] = toLitCond(VarE, 1^fCompE);
    pLits[2] = toLitCond(VarF, 0);
    if ( p->pPars->fPolarFlip )
    {
        if ( Gia_Regular(pNodeT)->fPhase )  pLits[0] = lit_neg( pLits[0] );
        if ( Gia_Regular(pNodeE)->fPhase )  pLits[1] = lit_neg( pLits[1] );
        if ( pNode->fPhase )                pLits[2] = lit_neg( pLits[2] );
    }
    RetValue = sat_solver_addclause( p->pSat, pLits, pLits + 3 );
    assert( RetValue );
}

/**Function*************************************************************

  Synopsis    [Addes clauses to the solver.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Cec_AddClausesSuper( Cec_ManSat_t * p, Gia_Obj_t * pNode, Vec_Ptr_t * vSuper )
{
    Gia_Obj_t * pFanin;
    int * pLits, nLits, RetValue, i;
    assert( !Gia_IsComplement(pNode) );
    assert( Gia_ObjIsAnd( pNode ) );
    // create storage for literals
    nLits = Vec_PtrSize(vSuper) + 1;
    pLits = ABC_ALLOC( int, nLits );
    // suppose AND-gate is A & B = C
    // add !A => !C   or   A + !C
    Vec_PtrForEachEntry( vSuper, pFanin, i )
    {
        pLits[0] = toLitCond(Cec_ObjSatNum(p,Gia_Regular(pFanin)), Gia_IsComplement(pFanin));
        pLits[1] = toLitCond(Cec_ObjSatNum(p,pNode), 1);
        if ( p->pPars->fPolarFlip )
        {
            if ( Gia_Regular(pFanin)->fPhase )  pLits[0] = lit_neg( pLits[0] );
            if ( pNode->fPhase )                pLits[1] = lit_neg( pLits[1] );
        }
        RetValue = sat_solver_addclause( p->pSat, pLits, pLits + 2 );
        assert( RetValue );
    }
    // add A & B => C   or   !A + !B + C
    Vec_PtrForEachEntry( vSuper, pFanin, i )
    {
        pLits[i] = toLitCond(Cec_ObjSatNum(p,Gia_Regular(pFanin)), !Gia_IsComplement(pFanin));
        if ( p->pPars->fPolarFlip )
        {
            if ( Gia_Regular(pFanin)->fPhase )  pLits[i] = lit_neg( pLits[i] );
        }
    }
    pLits[nLits-1] = toLitCond(Cec_ObjSatNum(p,pNode), 0);
    if ( p->pPars->fPolarFlip )
    {
        if ( pNode->fPhase )  pLits[nLits-1] = lit_neg( pLits[nLits-1] );
    }
    RetValue = sat_solver_addclause( p->pSat, pLits, pLits + nLits );
    assert( RetValue );
    ABC_FREE( pLits );
}

/**Function*************************************************************

  Synopsis    [Collects the supergate.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Cec_CollectSuper_rec( Gia_Obj_t * pObj, Vec_Ptr_t * vSuper, int fFirst, int fUseMuxes )
{
    // if the new node is complemented or a PI, another gate begins
    if ( Gia_IsComplement(pObj) || Gia_ObjIsCi(pObj) || 
         (!fFirst && Gia_ObjValue(pObj) > 1) || 
         (fUseMuxes && Gia_ObjIsMuxType(pObj)) )
    {
        Vec_PtrPushUnique( vSuper, pObj );
        return;
    }
    // go through the branches
    Cec_CollectSuper_rec( Gia_ObjChild0(pObj), vSuper, 0, fUseMuxes );
    Cec_CollectSuper_rec( Gia_ObjChild1(pObj), vSuper, 0, fUseMuxes );
}

/**Function*************************************************************

  Synopsis    [Collects the supergate.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Cec_CollectSuper( Gia_Obj_t * pObj, int fUseMuxes, Vec_Ptr_t * vSuper )
{
    assert( !Gia_IsComplement(pObj) );
    assert( !Gia_ObjIsCi(pObj) );
    Vec_PtrClear( vSuper );
    Cec_CollectSuper_rec( pObj, vSuper, 1, fUseMuxes );
}

/**Function*************************************************************

  Synopsis    [Updates the solver clause database.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Cec_ObjAddToFrontier( Cec_ManSat_t * p, Gia_Obj_t * pObj, Vec_Ptr_t * vFrontier )
{
    assert( !Gia_IsComplement(pObj) );
    if ( Cec_ObjSatNum(p,pObj) )
        return;
    assert( Cec_ObjSatNum(p,pObj) == 0 );
    if ( Gia_ObjIsConst0(pObj) )
        return;
    Vec_PtrPush( p->vUsedNodes, pObj );
    Cec_ObjSetSatNum( p, pObj, p->nSatVars++ );
    if ( Gia_ObjIsAnd(pObj) )
        Vec_PtrPush( vFrontier, pObj );
}

/**Function*************************************************************

  Synopsis    [Updates the solver clause database.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Cec_CnfNodeAddToSolver( Cec_ManSat_t * p, Gia_Obj_t * pObj )
{ 
    Vec_Ptr_t * vFrontier;
    Gia_Obj_t * pNode, * pFanin;
    int i, k, fUseMuxes = 1;
    // quit if CNF is ready
    if ( Cec_ObjSatNum(p,pObj) )
        return;
    if ( Gia_ObjIsCi(pObj) )
    {
        Vec_PtrPush( p->vUsedNodes, pObj );
        Cec_ObjSetSatNum( p, pObj, p->nSatVars++ );
        sat_solver_setnvars( p->pSat, p->nSatVars );
        return;
    }
    assert( Gia_ObjIsAnd(pObj) );
    // start the frontier
    vFrontier = Vec_PtrAlloc( 100 );
    Cec_ObjAddToFrontier( p, pObj, vFrontier );
    // explore nodes in the frontier
    Vec_PtrForEachEntry( vFrontier, pNode, i )
    {
        // create the supergate
        assert( Cec_ObjSatNum(p,pNode) );
        if ( fUseMuxes && Gia_ObjIsMuxType(pNode) )
        {
            Vec_PtrClear( p->vFanins );
            Vec_PtrPushUnique( p->vFanins, Gia_ObjFanin0( Gia_ObjFanin0(pNode) ) );
            Vec_PtrPushUnique( p->vFanins, Gia_ObjFanin0( Gia_ObjFanin1(pNode) ) );
            Vec_PtrPushUnique( p->vFanins, Gia_ObjFanin1( Gia_ObjFanin0(pNode) ) );
            Vec_PtrPushUnique( p->vFanins, Gia_ObjFanin1( Gia_ObjFanin1(pNode) ) );
            Vec_PtrForEachEntry( p->vFanins, pFanin, k )
                Cec_ObjAddToFrontier( p, Gia_Regular(pFanin), vFrontier );
            Cec_AddClausesMux( p, pNode );
        }
        else
        {
            Cec_CollectSuper( pNode, fUseMuxes, p->vFanins );
            Vec_PtrForEachEntry( p->vFanins, pFanin, k )
                Cec_ObjAddToFrontier( p, Gia_Regular(pFanin), vFrontier );
            Cec_AddClausesSuper( p, pNode, p->vFanins );
        }
        assert( Vec_PtrSize(p->vFanins) > 1 );
    }
    Vec_PtrFree( vFrontier );
}


/**Function*************************************************************

  Synopsis    [Recycles the SAT solver.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Cec_ManSatSolverRecycle( Cec_ManSat_t * p )
{
    int Lit;
    if ( p->pSat )
    {
        Gia_Obj_t * pObj;
        int i;
        Vec_PtrForEachEntry( p->vUsedNodes, pObj, i )
            Cec_ObjSetSatNum( p, pObj, 0 );
        Vec_PtrClear( p->vUsedNodes );
//        memset( p->pSatVars, 0, sizeof(int) * Aig_ManObjNumMax(p->pAigTotal) );
        sat_solver_delete( p->pSat );
    }
    p->pSat = sat_solver_new();
    sat_solver_setnvars( p->pSat, 1000 );
    // var 0 is not used
    // var 1 is reserved for const0 node - add the clause
    p->nSatVars = 1;
//    p->nSatVars = 0;
    Lit = toLitCond( p->nSatVars, 1 );
    if ( p->pPars->fPolarFlip )
        Lit = lit_neg( Lit );
    sat_solver_addclause( p->pSat, &Lit, &Lit + 1 );
    Cec_ObjSetSatNum( p, Gia_ManConst0(p->pAig), p->nSatVars++ );

    p->nRecycles++;
    p->nCallsSince = 0;
}

/**Function*************************************************************

  Synopsis    [Sets variable activities in the cone.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Cec_SetActivityFactors_rec( Cec_ManSat_t * p, Gia_Obj_t * pObj, int LevelMin, int LevelMax )
{
    float dActConeBumpMax = 20.0;
    int iVar;
    // skip visited variables
    if ( Gia_ObjIsTravIdCurrent(p->pAig, pObj) )
        return;
    Gia_ObjSetTravIdCurrent(p->pAig, pObj);
    // add the PI to the list
    if ( Gia_ObjLevel(p->pAig, pObj) <= LevelMin || Gia_ObjIsCi(pObj) )
        return;
    // set the factor of this variable
    // (LevelMax-LevelMin) / (pObj->Level-LevelMin) = p->pPars->dActConeBumpMax / ThisBump
    if ( (iVar = Cec_ObjSatNum(p,pObj)) )
    {
        p->pSat->factors[iVar] = dActConeBumpMax * (Gia_ObjLevel(p->pAig, pObj) - LevelMin)/(LevelMax - LevelMin);
        veci_push(&p->pSat->act_vars, iVar);
    }
    // explore the fanins
    Cec_SetActivityFactors_rec( p, Gia_ObjFanin0(pObj), LevelMin, LevelMax );
    Cec_SetActivityFactors_rec( p, Gia_ObjFanin1(pObj), LevelMin, LevelMax );
}

/**Function*************************************************************

  Synopsis    [Sets variable activities in the cone.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int Cec_SetActivityFactors( Cec_ManSat_t * p, Gia_Obj_t * pObj )
{
    float dActConeRatio = 0.5;
    int LevelMin, LevelMax;
    // reset the active variables
    veci_resize(&p->pSat->act_vars, 0);
    // prepare for traversal
    Gia_ManIncrementTravId( p->pAig );
    // determine the min and max level to visit
    assert( dActConeRatio > 0 && dActConeRatio < 1 );
    LevelMax = Gia_ObjLevel(p->pAig,pObj);
    LevelMin = (int)(LevelMax * (1.0 - dActConeRatio));
    // traverse
    Cec_SetActivityFactors_rec( p, pObj, LevelMin, LevelMax );
//Cec_PrintActivity( p );
    return 1;
}


/**Function*************************************************************

  Synopsis    [Runs equivalence test for the two nodes.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int Cec_ManSatCheckNode( Cec_ManSat_t * p, Gia_Obj_t * pObj )
{
    int nBTLimit = p->pPars->nBTLimit;
    int Lit, RetValue, status, clk, clk2, nConflicts;

    p->nCallsSince++;  // experiment with this!!!
    p->nSatTotal++;
    
    // check if SAT solver needs recycling
    if ( p->pSat == NULL || 
        (p->pPars->nSatVarMax && 
         p->nSatVars > p->pPars->nSatVarMax && 
         p->nCallsSince > p->pPars->nCallsRecycle) )
        Cec_ManSatSolverRecycle( p );

    // if the nodes do not have SAT variables, allocate them
clk2 = clock();
    Cec_CnfNodeAddToSolver( p, Gia_ObjFanin0(pObj) );
//ABC_PRT( "cnf", clock() - clk2 );
//printf( "%d \n", p->pSat->size );

clk2 = clock();
//    Cec_SetActivityFactors( p, Gia_ObjFanin0(pObj) ); 
//ABC_PRT( "act", clock() - clk2 );

    // propage unit clauses
    if ( p->pSat->qtail != p->pSat->qhead )
    {
        status = sat_solver_simplify(p->pSat);
        assert( status != 0 );
        assert( p->pSat->qtail == p->pSat->qhead );
    }

    // solve under assumptions
    // A = 1; B = 0     OR     A = 1; B = 1 
    Lit = toLitCond( Cec_ObjSatNum(p,Gia_ObjFanin0(pObj)), Gia_ObjFaninC0(pObj) );
    if ( p->pPars->fPolarFlip )
    {
        if ( Gia_ObjFanin0(pObj)->fPhase )  Lit = lit_neg( Lit );
    }
//Sat_SolverWriteDimacs( p->pSat, "temp.cnf", pLits, pLits + 2, 1 );
clk = clock();
    nConflicts = p->pSat->stats.conflicts;

clk2 = clock();
    RetValue = sat_solver_solve( p->pSat, &Lit, &Lit + 1, 
        (ABC_INT64_T)nBTLimit, (ABC_INT64_T)0, (ABC_INT64_T)0, (ABC_INT64_T)0 );
//ABC_PRT( "sat", clock() - clk2 );

    if ( RetValue == l_False )
    {
p->timeSatUnsat += clock() - clk;
        Lit = lit_neg( Lit );
        RetValue = sat_solver_addclause( p->pSat, &Lit, &Lit + 1 );
        assert( RetValue );
        p->nSatUnsat++;
        p->nConfUnsat += p->pSat->stats.conflicts - nConflicts;       
//printf( "UNSAT after %d conflicts\n", p->pSat->stats.conflicts - nConflicts );
        return 1;
    }
    else if ( RetValue == l_True )
    {
p->timeSatSat += clock() - clk;
        p->nSatSat++;
        p->nConfSat += p->pSat->stats.conflicts - nConflicts;
//printf( "SAT after %d conflicts\n", p->pSat->stats.conflicts - nConflicts );
        return 0;
    }
    else // if ( RetValue == l_Undef )
    {
p->timeSatUndec += clock() - clk;
        p->nSatUndec++;
        p->nConfUndec += p->pSat->stats.conflicts - nConflicts;
//printf( "UNDEC after %d conflicts\n", p->pSat->stats.conflicts - nConflicts );
        return -1;
    }
}


/**Function*************************************************************

  Synopsis    [Performs one round of solving for the POs of the AIG.]

  Description [Labels the nodes that have been proved (pObj->fMark1) 
  and returns the set of satisfying assignments.]
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Cec_ManSatSolve( Cec_ManPat_t * pPat, Gia_Man_t * pAig, Cec_ParSat_t * pPars )
{
    static int Counter;
//    char Buffer[1000];

    Bar_Progress_t * pProgress = NULL;
    Cec_ManSat_t * p;
    Gia_Obj_t * pObj;
    int i, status, clk = clock(), clk2;

//    sprintf( Buffer, "gia%03d.aig", Counter++ );
//Gia_WriteAiger( pAig, Buffer, 0, 0 );
//printf( "Dumpted slice into file \"%s\".\n", Buffer );

    // reset the manager
    if ( pPat )
    {
        pPat->iStart = Vec_StrSize(pPat->vStorage);
        pPat->nPats = 0;
        pPat->nPatLits = 0;
        pPat->nPatLitsMin = 0;
    } 
    Gia_ManSetPhase( pAig );
    Gia_ManLevelNum( pAig );
    Gia_ManResetTravId( pAig );
    p = Cec_ManSatCreate( pAig, pPars );
    pProgress = Bar_ProgressStart( stdout, Gia_ManPoNum(pAig) );
    Gia_ManForEachCo( pAig, pObj, i )
    {
        if ( Gia_ObjIsConst0(Gia_ObjFanin0(pObj)) )
        {
            pObj->fMark0 = 0;
            pObj->fMark1 = 1;
            continue;
        }
        Bar_ProgressUpdate( pProgress, i, "SAT..." );
clk2 = clock();
        status = Cec_ManSatCheckNode( p, pObj );
        pObj->fMark0 = (status == 0);
        pObj->fMark1 = (status == 1);
/*
printf( "Output %6d : ", i );
printf( "conf = %6d ", p->pSat->stats.conflicts );
printf( "prop = %6d ", p->pSat->stats.propagations );
ABC_PRT( "time", clock() - clk2 );
*/

/*
        if ( status == -1 )
        {
            Gia_Man_t * pTemp = Gia_ManDupDfsCone( pAig, pObj );
            Gia_WriteAiger( pTemp, "gia_hard.aig", 0, 0 );
            Gia_ManStop( pTemp );
            printf( "Dumping hard cone into file \"%s\".\n", "gia_hard.aig" );
        }
*/
        if ( status != 0 )
            continue;
        // save the pattern
        if ( pPat )
        {
            int clk3 = clock();
            Cec_ManPatSavePattern( pPat, p, pObj );
            pPat->timeTotalSave += clock() - clk3;
        }
        // quit if one of them is solved
        if ( pPars->fFirstStop )
            break;
    }
    p->timeTotal = clock() - clk;
    Bar_ProgressStop( pProgress );
    if ( pPars->fVerbose )
        Cec_ManSatPrintStats( p );
    Cec_ManSatStop( p );
}



/**Function*************************************************************

  Synopsis    [Save values in the cone of influence.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Cec_ManSatSolveSeq_rec( Cec_ManSat_t * pSat, Gia_Man_t * p, Gia_Obj_t * pObj, Vec_Ptr_t * vInfo, int iPat, int nRegs )
{
    if ( Gia_ObjIsTravIdCurrent(p, pObj) )
        return;
    Gia_ObjSetTravIdCurrent(p, pObj);
    if ( Gia_ObjIsCi(pObj) )
    {
        unsigned * pInfo = Vec_PtrEntry( vInfo, nRegs + Gia_ObjCioId(pObj) );
        if ( Cec_ObjSatVarValue( pSat, pObj ) != Aig_InfoHasBit( pInfo, iPat ) )
            Aig_InfoXorBit( pInfo, iPat );
        return;
    }
    assert( Gia_ObjIsAnd(pObj) );
    Cec_ManSatSolveSeq_rec( pSat, p, Gia_ObjFanin0(pObj), vInfo, iPat, nRegs );
    Cec_ManSatSolveSeq_rec( pSat, p, Gia_ObjFanin1(pObj), vInfo, iPat, nRegs );
}

/**Function*************************************************************

  Synopsis    [Performs one round of solving for the POs of the AIG.]

  Description [Labels the nodes that have been proved (pObj->fMark1) 
  and returns the set of satisfying assignments.]
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Cec_ManSatSolveSeq( Vec_Ptr_t * vPatts, Gia_Man_t * pAig, Cec_ParSat_t * pPars, int nRegs, int * pnPats )
{
    Bar_Progress_t * pProgress = NULL;
    Cec_ManSat_t * p;
    Gia_Obj_t * pObj;
    int iPat = 1, nPats = 32 * Vec_PtrReadWordsSimInfo(vPatts);
    int i, status, clk = clock();
    Gia_ManSetPhase( pAig );
    Gia_ManLevelNum( pAig );
    Gia_ManResetTravId( pAig );
    p = Cec_ManSatCreate( pAig, pPars );
    pProgress = Bar_ProgressStart( stdout, Gia_ManPoNum(pAig) );
    Gia_ManForEachCo( pAig, pObj, i )
    {
        if ( Gia_ObjIsConst0(Gia_ObjFanin0(pObj)) )
            continue;
        Bar_ProgressUpdate( pProgress, i, "BMC..." );
        status = Cec_ManSatCheckNode( p, pObj );
        if ( status != 0 )
            continue;
        // save the pattern
        Gia_ManIncrementTravId( pAig );
        Cec_ManSatSolveSeq_rec( p, pAig, Gia_ObjFanin0(pObj), vPatts, iPat++, nRegs );
        if ( iPat == nPats )
            break;
        // quit if one of them is solved
        if ( pPars->fFirstStop )
            break;
    }
    p->timeTotal = clock() - clk;
    Bar_ProgressStop( pProgress );
//    Cec_ManSatPrintStats( p );
    Cec_ManSatStop( p );
    if ( pnPats )
        *pnPats = iPat-1;
}

////////////////////////////////////////////////////////////////////////
///                       END OF FILE                                ///
////////////////////////////////////////////////////////////////////////


