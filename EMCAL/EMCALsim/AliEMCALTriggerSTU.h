#ifndef ALIEMCALTRIGGERSTU_H
#define ALIEMCALTRIGGERSTU_H
/* Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 * See cxx source for full Copyright notice                               */

/*

 
Author: R. GUERNANE LPSC Grenoble CNRS/IN2P3
*/

#include <AliEMCALTriggerTypes.h>
#include <AliEMCALTriggerBoard.h>

class TTree;
class AliEMCALTriggerSTUDCSConfig;

class AliEMCALTriggerSTU : public AliEMCALTriggerBoard 
{
public:
	
	              AliEMCALTriggerSTU();
	              AliEMCALTriggerSTU(AliEMCALTriggerSTUDCSConfig *dcsConf, const TVector2& rsize);
	virtual      ~AliEMCALTriggerSTU();
	
	virtual void  Build(TString& str, Int_t i, Int_t** Map, const TVector2* rSize, Int_t triggerMapping);
	virtual void  L1(int type);
	
	virtual void  ComputeThFromV0(int type, const Int_t M[]);
	
	virtual void  SetThreshold(int type, Int_t v);
	virtual Int_t GetThreshold(int type);
	virtual void  SetFw(int iFw) { fFw = iFw;}
	virtual Int_t GetFw() { return fFw;}

	virtual Int_t GetMedianEnergy();
	virtual void  SetBkgRho(Int_t fRho) { fBkgRho = fRho; }

	virtual void  Reset();
	
	virtual AliEMCALTriggerSTUDCSConfig* GetDCSConfig() const {return fDCSConfig;}

protected:
	
				  AliEMCALTriggerSTU(const AliEMCALTriggerSTU& rhs);
				  AliEMCALTriggerSTU& operator=(const AliEMCALTriggerSTU& rhs);

private:
	
		  Int_t   fGammaTh[2]; // Gamma threshold
		  Int_t   fJetTh[2];   // Jet threshold
			Int_t   fFw; // fW version	
			Int_t   fBkgRho; // BkgRho for L1 calculation.  Calculated from the other STU. 


	AliEMCALTriggerSTUDCSConfig *fDCSConfig; // DCS config

	ClassDef(AliEMCALTriggerSTU, 2)
};
 
#endif
