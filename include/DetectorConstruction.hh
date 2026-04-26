#ifndef DetectorConstruction_hh
#define DetectorConstruction_hh 1

#include "G4VUserDetectorConstruction.hh"
#include "globals.hh"

class G4VPhysicalVolume;
class G4LogicalVolume;
class LigLogVol;
class UrokoLogVol;
class HILELogVol;
class HPGeLogVol;
class MagnetLogVol;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

class DetectorConstruction : public G4VUserDetectorConstruction
{
  public:
    DetectorConstruction();
    virtual ~DetectorConstruction();

    virtual G4VPhysicalVolume* Construct();
    virtual void ConstructSDandField();

  private:
    // 各検出器クラスのインスタンス
    LigLogVol* fLig;
    UrokoLogVol* fUroko;
    HILELogVol* fHile;
    HPGeLogVol* fHPGe;
    MagnetLogVol* fMagnet;

    G4LogicalVolume* World_LogVol;
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif