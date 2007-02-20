//  **************************************************************************
//  * Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
//  *                                                                        *
//  * Author: The ALICE Off-line Project.                                    *
//  * Contributors are mentioned in the code where appropriate.              *
//  *                                                                        *
//  * Permission to use, copy, modify and distribute this software and its   *
//  * documentation strictly for non-commercial purposes is hereby granted   *
//  * without fee, provided that the above copyright notice appears in all   *
//  * copies and that both the copyright notice and this permission notice   *
//  * appear in the supporting documentation. The authors make no claims     *
//  * about the suitability of this software for any purpose. It is          *
//  * provided "as is" without express or implied warranty.                  *
//  **************************************************************************

#include "AliHMPIDCluster.h"  //class header
#include <TMinuit.h>         //Solve()
#include <TClonesArray.h>    //Solve()
#include <TMarker.h>         //Draw()
ClassImp(AliHMPIDCluster)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void AliHMPIDCluster::CoG()
{
// Calculates naive cluster position as a center of gravity of its digits.
// Arguments: none 
//   Returns: none
  
//  if(fDigs==0) return;                                //no digits in this cluster
  fX=fY=fQ=0;                                           //set cluster position to (0,0) to start to collect contributions
  Int_t maxQpad=-1,maxQ=-1;                             //to calculate the pad with the highest charge
  AliHMPIDDigit *pDig;
  for(Int_t iDig=0;iDig<fDigs->GetEntriesFast();iDig++){//digits loop
    pDig=(AliHMPIDDigit*)fDigs->At(iDig);               //get pointer to next digit
    Float_t q=pDig->Q();                                //get QDC 
    fX += pDig->LorsX()*q;fY +=pDig->LorsY()*q;         //add digit center weighted by QDC
    fQ+=q;                                              //increment total charge 
    if(q>maxQ) {maxQpad = pDig->Pad();maxQ=(Int_t)q;}   // to find pad with highest charge
  }//digits loop
  if ( fQ != 0 )   fX/=fQ;fY/=fQ;                       //final center of gravity
  
 
  CorrSin();                                            //correct it by sinoid   
  fCh=pDig->Ch();                                       //initialize chamber number
  fMaxQpad = maxQpad; fMaxQ=maxQ;                       //store max charge pad to the field
  fXi=fX+99; fYi=fY+99; fQi=fQ+99;                      //initial local max position is to be shifted artificially 
  fSt=kCoG;
}//CoG()
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void AliHMPIDCluster::CorrSin() 
{
// Correction of cluster x position due to sinoid, see HMPID TDR  page 30
// Arguments: none
//   Returns: none
  AliHMPIDDigit dig;dig.Manual1(Ch(),fX,fY);                                               //tmp digit to get it center
  Float_t x=fX-dig.LorsX();  
  fX+=3.31267e-2*TMath::Sin(2*TMath::Pi()/0.8*x)-2.66575e-3*TMath::Sin(4*TMath::Pi()/0.8*x)+2.80553e-3*TMath::Sin(6*TMath::Pi()/0.8*x)+0.0070;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void AliHMPIDCluster::Draw(Option_t*)
{
  TMarker *pMark=new TMarker(X(),Y(),5); pMark->SetMarkerColor(kBlue); pMark->Draw();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void AliHMPIDCluster::FitFunc(Int_t &iNpars, Double_t *, Double_t &chi2, Double_t *par, Int_t )
{
// Cluster fit function 
// par[0]=x par[1]=y par[2]=q for the first Mathieson shape
// par[3]=x par[4]=y par[5]=q for the second Mathieson shape and so on up to iNpars/3 Mathieson shapes
// For each pad of the cluster calculates the difference between actual pad charge and the charge induced to this pad by all Mathieson distributions 
// Then the chi2 is calculated as the sum of this value squared for all pad in the cluster.  
// Arguments: iNpars - number of parameters which is number of local maxima of cluster * 3
//            chi2   - function result to be minimised 
//            par   - parameters array of size iNpars            
//   Returns: none  
  AliHMPIDCluster *pClu=(AliHMPIDCluster*)gMinuit->GetObjectFit();
  Int_t iNshape = iNpars/3;
    
  chi2 = 0;
  for(Int_t i=0;i<pClu->Size();i++){                                       //loop on all pads of the cluster
    Double_t dQpadMath = 0;                                                //pad charge collector  
    for(Int_t j=0;j<iNshape;j++){                                          //Mathiesons loop as all of them may contribute to this pad
      dQpadMath+=par[3*j+2]*pClu->Dig(i)->Mathieson(par[3*j],par[3*j+1]);  // par[3*j+2] is charge par[3*j] is x par[3*j+1] is y of current Mathieson
    }
    chi2 +=TMath::Power((pClu->Dig(i)->Q()-dQpadMath),2);                  //
  }                                                                             //loop on all pads of the cluster     
}//FitFunction()
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void AliHMPIDCluster::Print(Option_t* opt)const
{
//Print current cluster  
  const char *status=0;
  switch(fSt){
    case        kFrm  : status="formed        "   ;break;
    case        kUnf  : status="unfolded (fit)"   ;break;
    case        kCoG  : status="coged         "   ;break;
    case        kLo1  : status="locmax 1 (fit)"   ;break;
    case        kAbn  : status="abnorm   (fit)"   ;break;
    case        kMax  : status="exceeded (cog)"   ;break;
    case        kNot  : status="not done (cog)"   ;break;
    case        kEmp  : status="empty         "   ;break;
    case        kEdg  : status="edge     (fit)"   ;break;
    case 	kSi1  : status="size 1   (cog)"   ;break;
    case 	kNoLoc: status="no LocMax(fit)"   ;break;
    
    default:            status="??????"          ;break;   
  }
  Printf("%sCLU:(%7.3f,%7.3f) Q=%8.3f  ch=%i, FormedSize=%2i   N loc. max. %i Box %i  Chi2 %7.3f        %s",
         opt,    X(),  Y(),   Q(),     Ch(),  Size(),           fNlocMax,       fBox,   fChi2,                 status);
  if(fDigs) fDigs->Print();    
}//Print()
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Int_t AliHMPIDCluster::Solve(TClonesArray *pCluLst,Bool_t isTryUnfold)
{
//This methode is invoked when the cluster is formed to solve it. Solve the cluster means to try to unfold the cluster
//into the local maxima number of clusters. This methode is invoked by AliHMPIDRconstructor::Dig2Clu() on cluster by cluster basis.  
//At this point, cluster contains a list of digits, cluster charge and size is precalculated in AddDigit(), position is preset to (-1,-1) in ctor,
//status is preset to kFormed in AddDigit(), chamber-sector info is preseted to actual values in AddDigit()
//Method first finds number of local maxima and if it's more then one tries to unfold this cluster into local maxima number of clusters
//Arguments: pCluLst     - cluster list pointer where to add new cluster(s)
//           isTryUnfold - flag to switch on/off unfolding   
//  Returns: number of local maxima of original cluster
  CoG();
  //  Printf("1 - fStatus: %d",fSt);
  Int_t iCluCnt=pCluLst->GetEntriesFast();                                             //get current number of clusters already stored in the list by previous operations
  if(isTryUnfold==kFALSE || Size()==1) {                                               //if cluster contains single pad there is no way to improve the knowledge 
    (isTryUnfold)?fSt=kSi1:fSt=kNot;
    new ((*pCluLst)[iCluCnt++]) AliHMPIDCluster(*this);  //add this raw cluster 
    return 1;
  } 
  // Printf("2 - fStatus: %d",fSt);
//Phase 0. Initialise TMinuit  
  const Int_t kMaxLocMax=6;                                                            //max allowed number of loc max for fitting
  TMinuit *pMinuit = new TMinuit(3*kMaxLocMax);                                        //init MINUIT with this number of parameters (3 params per mathieson)
  pMinuit->SetObjectFit((TObject*)this);  pMinuit->SetFCN(AliHMPIDCluster::FitFunc);   //set fit function
  Double_t aArg=-1;                                     Int_t iErrFlg;                 //tmp vars for TMinuit
  pMinuit->mnexcm("SET PRI",&aArg,1,iErrFlg);                                          //suspend all printout from TMinuit 
  pMinuit->mnexcm("SET NOW",&aArg,0,iErrFlg);                                          //suspend all warning printout from TMinuit
//Phase 1. Find number of local maxima. Strategy is to check if the current pad has QDC more then all neigbours. Also find the box contaning the cluster   
  fNlocMax=0;
  Int_t minPadX=999,minPadY=999,maxPadX=-1,maxPadY=-1,pc=-1;                             //for box finding   
  //Double_t  lowX,highX,lowY,highY;
  
  //  Printf("3 - fStatus: %d",fSt);
 for(Int_t iDig1=0;iDig1<Size();iDig1++) {                                              //first digits loop
    AliHMPIDDigit *pDig1 = Dig(iDig1);                                                   //take next digit
    pc=pDig1->Pc();                                                                      //finding the box  
    
    if(pDig1->PadPcX() > maxPadX) maxPadX = pDig1->PadPcX();                              
    if(pDig1->PadPcY() > maxPadY) maxPadY = pDig1->PadPcY();
    if(pDig1->PadPcX() < minPadX) minPadX = pDig1->PadPcX();
    if(pDig1->PadPcY() < minPadY) minPadY = pDig1->PadPcY();
    
    fBox=(maxPadX-minPadX+1)*100+maxPadY-minPadY+1;
    
    Int_t iHowManyMoreCnt = 0;                                                           //counts how many neighbouring pads has QDC more then current one
    for(Int_t iDig2=0;iDig2<Size();iDig2++) {                                            //loop on all digits again
      if(iDig1==iDig2) continue;                                                         //the same digit, no need to compare 
      AliHMPIDDigit *pDig2 = Dig(iDig2);                                                 //take second digit to compare with the first one
      Int_t dist = TMath::Sign(Int_t(pDig1->PadChX()-pDig2->PadChX()),1)+TMath::Sign(Int_t(pDig1->PadChY()-pDig2->PadChY()),1);//distance between pads
      if(dist==1)                                                                        //means dig2 is a neighbour of dig1
         if(pDig2->Q()>=pDig1->Q()) iHowManyMoreCnt++;                                   //count number of pads with Q more then Q of current pad
    }//second digits loop
    if(iHowManyMoreCnt==0&&fNlocMax<kMaxLocMax){                                       //this pad has Q more then any neighbour so it's local maximum
      
      /*
      lowX  = AliHMPIDDigit::LorsX(pc,minPadX) - 0.5 *AliHMPIDDigit::SizePadX();
      highX = AliHMPIDDigit::LorsX(pc,maxPadX) + 0.5 *AliHMPIDDigit::SizePadX();
      lowY  = AliHMPIDDigit::LorsY(pc,minPadY) - 0.5 *AliHMPIDDigit::SizePadY();
      highY = AliHMPIDDigit::LorsY(pc,maxPadY) + 0.5 *AliHMPIDDigit::SizePadY();
      */
      //Double_t    lowQ=0,highQ=30000; 
      
      fQi=pDig1->Q();  fXi=pDig1->LorsX();  fYi=pDig1->LorsY();                          //initial position of this Mathieson is to be in the center of loc max pad                               
      /*
        pMinuit->mnparm(3*fNlocMax  ,Form("x%i",fNlocMax),fXi,0.01,lowX,highX,iErrFlg);
        pMinuit->mnparm(3*fNlocMax+1,Form("y%i",fNlocMax),fYi,0.01,lowY,highY,iErrFlg);
        pMinuit->mnparm(3*fNlocMax+2,Form("q%i",fNlocMax),fQi,0.01,lowQ,highQ,iErrFlg);
      */
      pMinuit->mnparm(3*fNlocMax  ,Form("x%i",fNlocMax),fXi,0.01,0,0,iErrFlg);
      pMinuit->mnparm(3*fNlocMax+1,Form("y%i",fNlocMax),fYi,0.01,0,0,iErrFlg);
      pMinuit->mnparm(3*fNlocMax+2,Form("q%i",fNlocMax),fQi,0.01,0,100000,iErrFlg);
      
      fNlocMax++;
    }//if this pad is local maximum
  }//first digits loop
  
 //Int_t fitChk=0;

//Phase 2. Fit loc max number of Mathiesons or add this current cluster to the list
// Printf("4 - fStatus: %d",fSt);
 if ( fNlocMax == 0) { // case of no local maxima found: pads with same charge...
   pMinuit->mnparm(3*fNlocMax  ,Form("x%i",fNlocMax),fX,0.01,0,0,iErrFlg);
   pMinuit->mnparm(3*fNlocMax+1,Form("y%i",fNlocMax),fY,0.01,0,0,iErrFlg);
   pMinuit->mnparm(3*fNlocMax+2,Form("q%i",fNlocMax),fQ,0.01,0,100000,iErrFlg);
   fNlocMax = 1;
   fSt=kNoLoc;
 }
 
 if ( fNlocMax >= kMaxLocMax)
   {
     fSt   = kMax;   new ((*pCluLst)[iCluCnt++]) AliHMPIDCluster(*this);               //add this raw cluster  
   }
 else{                                                                                 //resonable number of local maxima to fit and user requested it
   Double_t arglist[10];     arglist[0] = 10000;     arglist[1] = 1.;                  //number of steps and sigma on pads charges  
   pMinuit->mnexcm("MIGRAD" ,arglist,0,iErrFlg);                                       //start fitting
   
   if (iErrFlg) 
     {
       fSt   = kAbn;                                                                   //fit fails, MINUIT returns error flag
       new ((*pCluLst)[iCluCnt++]) AliHMPIDCluster(*this);                             //add this raw cluster 
     }
   else
     {                                                                                 //Only if MIGRAD converged normally
       Double_t d2,d3; TString sName;                                                  //vars to get results from TMinuit
       for(Int_t i=0;i<fNlocMax;i++){                                                  //local maxima loop
         pMinuit->mnpout(3*i   ,sName,  fX, fXe , d2, d3, iErrFlg);
         pMinuit->mnpout(3*i+1 ,sName,  fY, fYe , d2, d3, iErrFlg);
         pMinuit->mnpout(3*i+2 ,sName,  fQ, fQe , d2, d3, iErrFlg);
         pMinuit->mnstat(fChi2,d2,d2,iErrFlg,iErrFlg,iErrFlg);
         
         if(fNlocMax!=1)fSt=kUnf;
         if(fNlocMax==1&&fSt!=kNoLoc) fSt=kLo1;
         if ( !IsInPc()) fSt = kEdg;       
         if(fSt==kNoLoc) fNlocMax=0;
         new ((*pCluLst)[iCluCnt++]) AliHMPIDCluster(*this);	   //add new unfolded cluster
       }
     }
 }
 
    

  

  delete pMinuit;
  return fNlocMax;
}//Solve()
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
