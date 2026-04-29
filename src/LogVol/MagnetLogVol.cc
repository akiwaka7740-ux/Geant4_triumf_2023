#include "Material/util/common.hh"

#include "LogVol/MagnetLogVol.hh"
#include "Material/NeomaxMat.hh"


#include "G4VSolid.hh"
#include "G4Sphere.hh"
#include "G4Box.hh"
#include "G4Cons.hh"
#include "G4Tubs.hh"
#include "G4Polycone.hh"
#include "G4UnionSolid.hh"
#include "G4IntersectionSolid.hh"
#include "G4SubtractionSolid.hh"
#include "G4SystemOfUnits.hh"

#include "G4NistManager.hh"
#include "G4PVPlacement.hh"
#include "G4VisAttributes.hh"

namespace MagnetUtil {
  
  enum Axis { X, Y, Z };
  G4Transform3D Move(double dx, double dy, double dz) {
    return G4Transform3D( G4RotationMatrix(), G4ThreeVector( dx,dy,dz ) );
  }
  G4Transform3D Rotate(int axis, double angle) {
    G4RotationMatrix rot;
    if( axis==Axis::X ) rot.rotateX( angle );
    if( axis==Axis::Y ) rot.rotateY( angle );
    if( axis==Axis::Z ) rot.rotateZ( angle );
    return G4Transform3D( rot, G4ThreeVector( 0,0,0 ) );
  }
  G4Transform3D Transform(int axis, double angle, double dx, double dy, double dz) {
    G4RotationMatrix rot;
    if( axis==Axis::X ) rot.rotateX( angle );
    if( axis==Axis::Y ) rot.rotateY( angle );
    if( axis==Axis::Z ) rot.rotateZ( angle );
    return G4Transform3D( rot, G4ThreeVector( dx, dy, dz ) );
  }

  enum class ColID {
    White,
    LightGrey,
    LightGray,
    Gray,
    Grey,
    Black,
    Brown,
    Red,
    Green,
    Blue,
    Cyan,
    Magenta,
    Yellow,
    DarkGrey,
  };

  struct RGB { double R; double G; double B; };
  static std::map<ColID, RGB> defaultRGB = {
    { ColID::White,    { 1.0, 1.0, 1.0 } },
    { ColID::LightGrey, { 0.7, 0.7, 0.7 } },
    { ColID::LightGray, { 0.7, 0.7, 0.7 } },
    { ColID::Gray,     { 0.5, 0.5, 0.5 } },
    { ColID::Grey,     { 0.5, 0.5, 0.5 } },
    { ColID::Black,    { 0.0, 0.0, 0.0 } },
    { ColID::Brown,    { 0.45,0.25,0.0 } },
    { ColID::Red,      { 1.0, 0.0, 0.0 } },
    { ColID::Green,    { 0.0, 1.0, 0.0 } },
    { ColID::Blue,     { 0.0, 0.0, 1.0 } },
    { ColID::Cyan,     { 0.0, 1.0, 1.0 } },
    { ColID::Magenta,  { 1.0, 0.0, 1.0 } },
    { ColID::Yellow ,  { 1.0, 1.0, 0.0 } },
    { ColID::DarkGrey, { 0.25,0.25,0.25} },
  };

  G4Colour Color(ColID ID, double Opacity=1) {
    return G4Colour( 
        defaultRGB[ID].R,
        defaultRGB[ID].G,
        defaultRGB[ID].B,
        Opacity );
  }
};
using namespace MagnetUtil;
MagnetLogVol::MagnetLogVol(G4String Name, G4UserLimits* fStepLimit, G4bool checkOverlaps)
{
  if(logmode) G4cout << "-- MagnetLogVol::MagnetLogVol(G4String)\n";

  //方針：磁石を左右に分割して複製することで全体を作成する

  // =============================================================
  // 1. Solid の作成
  // =============================================================

  Solid = new G4Box(Name+"_Solid", 500.0*mm/2.0, 500.0*mm/2.0 , 500.0*mm/2.0); // 十分大きな箱を作成

  //1.1. 磁石の前部分を定義
  const G4int numZ_Mag = 8;
  G4double zPlane_Magnet[] = {
      0.0*mm,   5.0*mm,   // 第1段
      5.0*mm,  10.0*mm,   // 第2段
     10.0*mm,  20.0*mm,   // 第3段
     20.0*mm,  45.0*mm    // 第4段
  };
  
  G4double rInner_Mag[] = {
      40.0/2.0*mm,  40.0/2.0*mm,   // 第1段 (半径20)
      60.0/2.0*mm,  60.0/2.0*mm,   // 第2段 (半径30)
      80.0/2.0*mm,  80.0/2.0*mm,   // 第3段 (半径40)
     100.0/2.0*mm, 100.0/2.0*mm    // 第4段 (半径50)
  };

  G4double rOuter_Mag[] = {
     122.0/2.0*mm, 122.0/2.0*mm, 122.0/2.0*mm, 122.0/2.0*mm,
     122.0/2.0*mm, 122.0/2.0*mm, 122.0/2.0*mm, 122.0/2.0*mm
  };

  G4VSolid* solid_Magnet = new G4Polycone(Name+"_Magnet_Solid", 0, 360*deg, numZ_Mag, zPlane_Magnet, rInner_Mag, rOuter_Mag);

  //1.2. 磁石の後半部分を定義
  const G4int numZ_Back = 4;
  G4double zPlane_Back[] = {
      0.0*mm,  15.0*mm,   // 第1段
      15.0*mm, 25.0*mm    // 第2段
  };

    G4double rInner_Back[] = {
        100.0/2.0*mm, 100.0/2.0*mm,  // 第1段 (半径50)
        100.0/2.0*mm, 122.0/2.0*mm   // 第2段 (半径50→61)
    };
  
    G4double rOuter_Back[] = {
        122.0/2.0*mm, 122.0/2.0*mm,  // 第1段 (半径61)
        122.0/2.0*mm, 122.0/2.0*mm   // 第2段 (半径61)
    };

    G4VSolid* solid_Back = new G4Polycone(Name+"_Back_Solid", 0, 360*deg, numZ_Back, zPlane_Back, rInner_Back, rOuter_Back);

    //1.3. ボディ0１
    G4VSolid *tmp01 = new G4Tubs(Name+"_tmp01", 0, 122.0/2.0*mm, 30.0*mm/2.0, 0, 360*deg); // 十分大きな円柱を作成
    G4VSolid *tmp02 = new G4Box(Name+"_tmp02", (132.0 + 145.0)/2.0*mm, 132.0/2.0*mm, 25.0*mm/2.0);

    G4double tmp_xshift = (132.0 + 145.0 - 122.0)/2.0*mm;
    G4ThreeVector translation_tmp01(tmp_xshift, 0, 0);

    G4VSolid* solid_Body01 = new G4SubtractionSolid(Name+"_Body01", tmp02, tmp01, 0, translation_tmp01);

    //1.4 ボディ02
    G4double x_tmp03 = 60.0 * mm /2.0;
    G4double x_tmp04 = 78.0 * mm /2.0;
    G4double x_tmp05 = 140.0 * mm /2.0;

    G4double y_tmp03 = 66.0 * mm /2.0;
    G4double y_tmp04 = 132.0 * mm /2.0;
    G4double y_tmp05 = 132.0 * mm /2.0;

    G4double z_tmp03 = 30.0 * mm /2.0;
    G4double z_tmp04 = 43.6 * mm /2.0;
    G4double z_tmp05 = z_tmp03 + z_tmp04;



    G4VSolid *tmp03 = new G4Box(Name+"_tmp03", x_tmp03, y_tmp03, z_tmp03 + 1.0*mm); // 十分大きな箱を作成 (z方向は余裕を持たせる)
    G4VSolid *tmp04 = new G4Box(Name+"_tmp04", x_tmp04, y_tmp04, z_tmp04 + 1.0*mm);
    G4VSolid *tmp05 = new G4Box(Name+"_tmp05", x_tmp05, y_tmp05, z_tmp05);
    
    G4ThreeVector translation_tmp03(0, 0, -(z_tmp05-z_tmp03));
    G4ThreeVector translation_tmp04(0, 0, (z_tmp05-z_tmp04));

    // 1. tmp05 から tmp03 を差し引く
    G4VSolid* tmp_sub = new G4SubtractionSolid(
        Name + "_sub1",
        tmp05,              // 土台
        tmp03,              // 削るもの1
        0,                  // 回転なし
        translation_tmp03   // tmp03の位置
    );

    // 2. 上記の結果（tmp_sub）から さらに tmp04 を差し引く
    G4VSolid* body02 = new G4SubtractionSolid(
        Name + "_body02",
        tmp_sub,            // 1段目で削り終わった形状
        tmp04,              // 削るもの2
        0,                  // 回転なし
        translation_tmp04   // tmp04の位置
    );


  // =============================================================
  // 2. Material と Logical Volume
  // =============================================================
    NeomaxMat* neomaxMat = new NeomaxMat();
    G4NistManager* nist = G4NistManager::Instance();
    
    G4Material* matNeomax = neomaxMat->GetMaterial();
    G4Material* matYoke = nist->FindOrBuildMaterial("G4_Fe");
    G4Material* matVac = nist->FindOrBuildMaterial("G4_Galactic");

    LogVol = new G4LogicalVolume(Solid, matVac, Name+"_LogVol", 0, 0, fStepLimit);
    LogVol->SetVisAttributes(G4VisAttributes::GetInvisible()); // マザーは透明に

    G4LogicalVolume* LogVol_Mag = new G4LogicalVolume(solid_Magnet, matNeomax, Name+"_Mag_LV", 0, 0, fStepLimit);
    G4LogicalVolume* LogVol_Back = new G4LogicalVolume(solid_Back, matYoke, Name+"_Back_LV", 0, 0, fStepLimit);
    G4LogicalVolume* LogVol_Body01 = new G4LogicalVolume(solid_Body01, matYoke, Name+"_Body01_LV", 0, 0, fStepLimit);
    G4LogicalVolume* LogVol_Body02 = new G4LogicalVolume(body02, matYoke, Name+"_Body02_LV", 0, 0, fStepLimit);

  // 色を見やすく設定   
    
    G4VisAttributes* visMag = new G4VisAttributes(TRUE, Color(ColID::Brown)); // ダークグレー
    visMag->SetForceSolid(true);
    LogVol_Mag->SetVisAttributes(visMag);   

    G4VisAttributes* visBack = new G4VisAttributes(TRUE, Color(ColID::Grey)); // グレー
    visBack->SetForceSolid(true);
    LogVol_Back->SetVisAttributes(visBack);

    G4VisAttributes* visBody01 = new G4VisAttributes(TRUE, Color(ColID::DarkGrey)); // 明るいグレー
    visBody01->SetForceSolid(true);
    LogVol_Body01->SetVisAttributes(visBody01);

    G4VisAttributes* visBody02 = new G4VisAttributes(TRUE, Color(ColID::Blue)); // 青
    visBody02->SetForceSolid(true);
    LogVol_Body02->SetVisAttributes(visBody02);

  // =============================================================
  // 3. Physical Volume (内部配置)
  // =============================================================

  // Z軸上の相対位置（高さ方向の積み重ね）

    double h_Mag = 45.0*mm; // 磁石の高さ
    double h_Back = 15.0*mm; // 磁石の後半部分の高さ (正確な寸法はないので仮の値)


    double z_Mag = 49.0*mm/2.0; // 磁石の前面の位置
    double z_Back = 49.0*mm/2.0 + h_Mag; // 磁石後半部分の前面の位置 (磁石の前面から積み重ねる形で配置)

    double x_Body01 = - (132.0 + 145.0 - 122.0)/2.0*mm; // Body01のX方向の位置 (tmp_xshiftと同じ値)
    double z_Body01 = 49.0*mm/2.0 + h_Mag + 25.0*mm/2.0; // Body01のZ方向の位置 (磁石の前面から積み重ねる形で配置)

    double x_Body02 = -(132.0*mm/2.0 + 70.4*mm + (43.6 + 30.0)/2.0); // Body02のZ方向の位置 (Body01の前面から積み重ねる形で配置)

    

  //各パーツをBox内に配置して、組み合わせることで全体のSolidを作成

    G4Transform3D trans_Magnet01 = Move(0, 0, z_Mag);
    G4Transform3D trans_Back01 = Move(0, 0, z_Back);
    G4Transform3D trans_Body01_01 = Move(x_Body01, 0, z_Body01);

    // 磁石を反転させて配置 
    G4Transform3D trans_Magnet02 = Move(0, 0, -z_Mag) * Rotate(Axis::Y, 180*deg); 
    G4Transform3D trans_Back02 = Move(0, 0, -z_Back) * Rotate(Axis::Y, 180*deg);
    G4Transform3D trans_Body01_02 = Move(x_Body01, 0, -z_Body01) ;

    G4Transform3D trans_Body02 = Move(x_Body02, 0, 0)  * Rotate(Axis::Y, 90*deg);


    
    new G4PVPlacement(trans_Magnet01, LogVol_Mag,"_Mag_01", LogVol, false, 0, checkOverlaps);
    new G4PVPlacement(trans_Back01, LogVol_Back,"_Back_01", LogVol, false, 0, checkOverlaps);
    new G4PVPlacement(trans_Body01_01, LogVol_Body01,"_Body01_01", LogVol, false, 0, checkOverlaps);

    new G4PVPlacement(trans_Magnet02, LogVol_Mag,"_Mag_02", LogVol, false, 0, checkOverlaps);
    new G4PVPlacement(trans_Back02, LogVol_Back,"_Back_02", LogVol, false, 0, checkOverlaps);   
    new G4PVPlacement(trans_Body01_02, LogVol_Body01,"_Body01_02", LogVol, false, 0, checkOverlaps);
    

    //new G4PVPlacement(trans_Body02, LogVol_Body02,"_Body02", LogVol, false, 0, checkOverlaps);  



}

MagnetLogVol::~MagnetLogVol() 
{
  if(logmode) G4cout << "-- MagnetLogVol::~MagnetLogVol()\n";
  delete Solid;
  delete LogVol;
  if(logmode) G4cout << "== MagnetLogVol::~MagnetLogVol()\n";
}
