#include "DetectorConstruction.hh"

DetectorConstruction::DetectorConstruction()
{
}

DetectorConstruction::~DetectorConstruction()
{
}

G4VPhysicalVolume *DetectorConstruction::Construct() {

    G4bool checkOverlaps = true;

    G4NistManager *nist = G4NistManager::Instance();
    G4Material *matAir = nist->FindOrBuildMaterial("G4_AIR"); // worldMat から matAir に変更

    // 1. 素材の読み込み
    // Isotopeの定義 (名前, Z, A, モル質量)
    G4Element* elO  = nist->FindOrBuildElement("O");
    G4Element* elSi = nist->FindOrBuildElement("Si");
    G4Element* elMg = nist->FindOrBuildElement("Mg");
    G4Element* elAl = nist->FindOrBuildElement("Al");
    G4Element* elCe = nist->FindOrBuildElement("Ce");       

    // 2. 濃縮リチウム (6Li 95% enrichment) の定義
    G4Isotope* iso_Li6 = new G4Isotope("Li6", 3, 6, 6.015*g/mole);
    G4Isotope* iso_Li7 = new G4Isotope("Li7", 3, 7, 7.016*g/mole);

    // カスタムElementの作成
    G4Element* elLi_enriched = new G4Element("Enriched_Lithium", "Li", 2);
    // アバンダンス（原子数比）で追加
    elLi_enriched->AddIsotope(iso_Li6, 95.0*perCent);
    elLi_enriched->AddIsotope(iso_Li7,  5.0*perCent);

    // 3. 各構成化合物 (酸化物) の定義
    G4Material* SiO2 = new G4Material("SiO2", 2.20*g/cm3, 2);
    SiO2->AddElement(elSi, 1);
    SiO2->AddElement(elO,  2);

    G4Material* MgO = new G4Material("MgO", 3.58*g/cm3, 2);
    MgO->AddElement(elMg, 1);
    MgO->AddElement(elO,  1);

    G4Material* Al2O3 = new G4Material("Al2O3", 3.95*g/cm3, 2);
    Al2O3->AddElement(elAl, 2);
    Al2O3->AddElement(elO,  3);

    G4Material* Ce2O3 = new G4Material("Ce2O3", 6.20*g/cm3, 2);
    Ce2O3->AddElement(elCe, 2);
    Ce2O3->AddElement(elO,  3);

    G4Material* Li2O = new G4Material("Li2O", 2.01*g/cm3, 2);
    Li2O->AddElement(elLi_enriched, 2);
    Li2O->AddElement(elO,  1);

    // 4. Liガラスシンチレータ (GS20) の完成
    G4double density_GS20 = 2.5 * g/cm3;
    G4Material* matGS20 = new G4Material("GS20", density_GS20, 5);

    // weight % に従って化合物を混合 (FractionMass)
    matGS20->AddMaterial(SiO2,  58.5 * perCent);
    matGS20->AddMaterial(MgO,    4.0 * perCent);
    matGS20->AddMaterial(Al2O3, 18.0 * perCent);
    matGS20->AddMaterial(Ce2O3,  4.0 * perCent);
    matGS20->AddMaterial(Li2O,  15.5 * perCent);

    // =============================================================
    // World Volume
    // =============================================================
    G4double xWorld = 2. * m;
    G4double yWorld = 2. * m;
    G4double zWorld = 2. * m;
    
    G4Box *SL_World = new G4Box("SL_World", 0.5*xWorld, 0.5*yWorld, 0.5*zWorld);
    G4LogicalVolume *LV_World = new G4LogicalVolume(SL_World, matAir, "LV_World"); // solidWorldのスペルミスを修正
    // PVPlacementのコピー番号(第6引数)は0にしておくのが一般的です
    G4VPhysicalVolume *PV_World = new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), LV_World, "PV_World", 0, false, 0, checkOverlaps);

    
    // =============================================================
    // Lig Volume (Li-glass Scintillator)
    // =============================================================
    G4double diameterLig = 50.0 * mm;
    G4double thicknessLig = 10.0 * mm;

    G4Tubs* SL_Lig = new G4Tubs(
        "SL_Lig", 
        0.0 * mm,                      
        diameterLig / 2.0,        
        thicknessLig / 2.0,       
        0.0 * deg,                     
        360.0 * deg                    
    );

    G4RotationMatrix* rotLig = new G4RotationMatrix();
    // 円柱の軸(Z)をY軸に向けるため、X軸周りに90度回転させる
    rotLig->rotateX(90.0 * deg);

    G4double xLig = 0.0 * mm;
    G4double frontLig = -132.0 * mm; // 検出器前面の位置
    // 中心位置は、前面からさらに「厚さの半分」だけ負の方向へ進んだ場所
    G4double yLig = frontLig - (thicknessLig / 2.0); 
    G4double zLig = 0.0 * mm;

    // ヘッダーファイル(DetectorConstruction.hh)側の変数名も fLV_Lig に変更する必要があります
    fLV_Lig = new G4LogicalVolume(SL_Lig, matGS20, "LV_Lig");
    
    // 第5引数(母体積)は LV_World。コピー番号の 0 も明記。
    G4VPhysicalVolume *PV_Lig = new G4PVPlacement(rotLig, G4ThreeVector(xLig, yLig, zLig), fLV_Lig, "PV_Lig", LV_World, false, 0, checkOverlaps);

    // 可視化属性の変数名も lig に統一
    G4VisAttributes *ligVisAtt = new G4VisAttributes(G4Color(1.0, 1.0, 0.0, 0.5));
    ligVisAtt->SetForceSolid(true);
    fLV_Lig->SetVisAttributes(ligVisAtt);

    return PV_World;
}

void DetectorConstruction::ConstructSDandField()
{
    SensitiveDetector *sensDet = new SensitiveDetector("SensitiveDetector");
    fLV_Lig->SetSensitiveDetector(sensDet);
    G4SDManager::GetSDMpointer()->AddNewDetector(sensDet);
}