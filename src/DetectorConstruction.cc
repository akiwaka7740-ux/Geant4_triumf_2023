#include "DetectorConstruction.hh"
#include "LogVol/LigLogVol.hh"
#include "LogVol/UrokoLogVol.hh"

#include "G4RunManager.hh"
#include "G4NistManager.hh"
#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"
#include "G4VisAttributes.hh"
#include "G4SDManager.hh"
#include "SensitiveDetector.hh"


DetectorConstruction::DetectorConstruction()
: G4VUserDetectorConstruction(), fLig(nullptr), fUroko(nullptr), World_LogVol(nullptr)
{}

DetectorConstruction::~DetectorConstruction()
{}

G4VPhysicalVolume* DetectorConstruction::Construct()
{
  G4bool checkOverlaps = true;
  G4NistManager* nist = G4NistManager::Instance();
  G4Material* matAir = nist->FindOrBuildMaterial("G4_AIR");

  // =============================================================
  // World Volume
  // =============================================================
  G4double world_size = 4.0 * m;
  G4Box* World_Solid = new G4Box("World_Solid", world_size/2., world_size/2., world_size/2.);
  World_LogVol = new G4LogicalVolume(World_Solid, matAir, "World_LogVol");
  World_LogVol->SetVisAttributes(new G4VisAttributes(TRUE, G4Colour(1.0, 1.0, 1.0, 0.0)));
  G4VPhysicalVolume* World_PhysVol = new G4PVPlacement(0, G4ThreeVector(), World_LogVol, "World_PhysVol", 0, false, 0, checkOverlaps);

  // =============================================================
  // LigGlass Detector 配置
  // =============================================================
  fLig = new LigLogVol("LigGlass", 0, checkOverlaps);
  G4LogicalVolume* Lig_LogVol = fLig->GetLogicalVolume();

  G4double front_Lig = -132.0 * mm;
  G4double thickness_Lig = 10.0 * mm;
  G4double total_length_Lig = 246.0 * mm;
  
  G4RotationMatrix rot_Lig;
  rot_Lig.rotateX(-90.0 * deg); // 前面(シンチ)を原点方向へ
  G4ThreeVector pos_Lig(0.0, front_Lig - (total_length_Lig / 2.0), 0.0);

  new G4PVPlacement(G4Transform3D(rot_Lig, pos_Lig), "LigGlass_Phys", Lig_LogVol, World_PhysVol, false, 0, checkOverlaps);

  

  // =============================================================
  // UROKO Detector 配置 (4台)
  // =============================================================
  fUroko = new UrokoLogVol("Uroko", 0, checkOverlaps);
  G4LogicalVolume* Uroko_LogVol = fUroko->GetLogicalVolume();

  // 配置用のパラメータ (元の設計値を利用)
  const G4int numUroko = 4;
  G4double updownDeg = 5.7 * deg;
  G4double sideDeg = 10.2 * deg;
  G4double margin = 2.5 * mm;
  G4double hexagon_r = 100.0 * mm;
  G4double hexagon_rr = hexagon_r * std::sqrt(3.0) / 2.0;
  G4double thickness_Uroko = 29.0 * mm;
  G4double total_Z_Uroko = 111.5 * mm; // Scinti + Guide + PMT全長の合計

  // 中心を軸に回転した際の、前面の「せり出し量」を三角関数で計算
  G4double swing_updown = (total_Z_Uroko / 2.0) * std::sin(updownDeg);
  G4double swing_side   = (total_Z_Uroko / 2.0) * std::sin(sideDeg);

  // 各UROKOの中心座標を計算
  G4ThreeVector pos_Uroko[numUroko] = {
      G4ThreeVector(1000*mm,  (hexagon_rr + margin + swing_updown), 0), // 上
      G4ThreeVector(1000*mm, -(hexagon_rr + margin + swing_updown), 0), // 下
      G4ThreeVector(1000*mm, 0,  (1.5 * hexagon_r + margin + swing_side)), // 左
      G4ThreeVector(1000*mm, 0, -(1.5 * hexagon_r + margin + swing_side))  // 右
  };

  // 配置中心の補正（シンチ前面を計算上の基準点にするため、厚みの半分と全長の半分でオフセット）
  for(int i=0; i<numUroko; i++) {
    pos_Uroko[i].setX(pos_Uroko[i].getX() + (total_Z_Uroko / 2.0)); 
  }

  G4RotationMatrix* rot_Uroko[numUroko];
  for(int i=0; i<numUroko; i++) rot_Uroko[i] = new G4RotationMatrix();

  rot_Uroko[0]->rotateY(90.0*deg); rot_Uroko[0]->rotateZ(updownDeg);
  rot_Uroko[1]->rotateY(90.0*deg); rot_Uroko[1]->rotateZ(-updownDeg);
  rot_Uroko[2]->rotateY(90.0*deg - sideDeg);
  rot_Uroko[3]->rotateY(90.0*deg + sideDeg);

  for(G4int i=0; i<numUroko; i++) {
    new G4PVPlacement(G4Transform3D(*rot_Uroko[i], pos_Uroko[i]), "Uroko_Phys", Uroko_LogVol, World_PhysVol, false, i, checkOverlaps);
  }

  return World_PhysVol;
}

void DetectorConstruction::ConstructSDandField()
{
  SensitiveDetector* sensDet = new SensitiveDetector("SensitiveDetector");
  G4SDManager::GetSDMpointer()->AddNewDetector(sensDet);

  if(fLig) fLig->GetScintiVolume()->SetSensitiveDetector(sensDet);

}