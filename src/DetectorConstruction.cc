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


    //for uroko
    auto BC408 = nist->FindOrBuildMaterial("G4_PLASTIC_SC_VINYLTOLUENE");
    auto acrylic = nist->FindOrBuildMaterial("G4_PLEXIGLASS");
    auto aluminum = nist->FindOrBuildMaterial("G4_Al");
    auto vaccum = nist->FindOrBuildMaterial("G4_Galactic");
    auto glass = nist->FindOrBuildMaterial("G4_Pyrex_Glass");


    // =============================================================
    // World Volume
    // =============================================================
    G4double xWorld = 4. * m;
    G4double yWorld = 4. * m;
    G4double zWorld = 4. * m;
    
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




    // =============================================================
    // 1. UROKOジオメトリの寸法と変数の定義
    // =============================================================

    // --- 型の略称定義（エイリアス） ---
    using GVC = G4ThreeVector;
    using GTR = G4Transform3D;

    // --- 回転行列の定義 ---
    // Y軸周りに90度回転する行列を作成
    G4RotationMatrix rot90y;
    rot90y.rotateY(90.0 * deg);

    G4int Mother_ID_Uroko = 1;
    G4int Scinti_ID_Uroko = 2;
    G4int Guide_ID_Uroko = 3;
    G4int PMT_ID_Uroko = 4;
    G4int Cathode_ID_Uroko = 5;
    G4int PMT2_ID_Uroko = 6;
    G4int Cathode2_ID_Uroko = 7;

    // --- UROKO 寸法パラメータ ---
    G4double thickness_Uroko = 29 * mm;
    G4double PMT_W_Uroko = 26.2 * mm;
    G4double PMT_L_Uroko = 32.5 * mm;
    G4double PMT_C_Uroko = 30 * mm;
    G4double cathode_W_Uroko = 23 * mm;
    G4double cathode_T_Uroko = 0.8 * mm;
    G4double guide_L_Uroko = 40 * mm;
    G4double guide_S_Uroko = 10 * mm;
    G4double flight_length_Uroko = 1.5 * m;
    G4double hexagon_r_Uroko = 100 * mm;
    G4double hexagon_rr_Uroko = hexagon_r_Uroko * std::sqrt(3) / 2 * mm; 
    G4double hexagon_length_Uroko = std::sqrt(1500 * 1500 - hexagon_r_Uroko * hexagon_r_Uroko) * mm;

    // ポリゴン（多角柱）用配列
    G4double z1_Uroko[] = {0, thickness_Uroko};
    G4double rI1_Uroko[] = {0, 0};
    G4double rO1_Uroko[] = {hexagon_rr_Uroko, hexagon_rr_Uroko};
    G4double z2_Uroko[] = {-guide_L_Uroko / 2, guide_L_Uroko / 2};
    G4double PMT_W2_Uroko = (PMT_W_Uroko + ((PMT_C_Uroko - PMT_W_Uroko) / 2)) * 2;
    G4double rO2_Uroko[] = {hexagon_rr_Uroko, (PMT_W2_Uroko * std::sqrt(3) + PMT_W_Uroko) / 4};


    // =============================================================
    // 2. UROKO Mother Volume (アセンブリを使用)
    // =============================================================

    G4AssemblyVolume* assembly_Uroko = new G4AssemblyVolume();
    
    /*
    G4Box *SL_Mother_Uroko = new G4Box("SL_Mother_Uroko", hexagon_r_Uroko * 1.1, hexagon_r_Uroko * 1.1, (thickness_Uroko + guide_L_Uroko + guide_S_Uroko + PMT_L_Uroko) * 1.1);
    G4LogicalVolume *LV_Mother_Uroko = new G4LogicalVolume(SL_Mother_Uroko, LV_World->GetMaterial(), "LV_Mother_Uroko", 0, 0, 0);

    // 注意: rot90y, GTR, GVC は元のコードのエイリアス/変数をそのまま使用
    G4VPhysicalVolume *PV_Mother_Uroko = new G4PVPlacement(GTR(rot90y, GVC(flight_length_Uroko, 0, 0)), LV_Mother_Uroko, "PV_Mother_Uroko", LV_World, false, Mother_ID_Uroko, checkOverlaps);

    //　光学過程を考える際に、検出器前面での反射などを光量するために設定、中身はただの空気
    G4Polyhedra *SL_Mother2_Uroko = new G4Polyhedra("SL_Mother2_Uroko", 0 * degree, 360 * degree, 6, 2, z1_Uroko, rI1_Uroko, rO1_Uroko);
    G4LogicalVolume *LV_Mother2_Uroko = new G4LogicalVolume(SL_Mother2_Uroko, LV_World->GetMaterial(), "LV_Mother2_Uroko", 0, 0, 0);

    auto rotation_1_Uroko = G4RotationMatrix(0., 0., 0.);
    auto position_1_Uroko = G4ThreeVector(0, 0, hexagon_length_Uroko - flight_length_Uroko - thickness_Uroko);
    auto trans3D_1_Uroko = G4Transform3D(rotation_1_Uroko, position_1_Uroko);
    G4VPhysicalVolume *PV_Mother2_Uroko = new G4PVPlacement(trans3D_1_Uroko, LV_Mother2_Uroko, "PV_Mother2_Uroko", LV_Mother_Uroko, false, Mother_ID_Uroko, checkOverlaps);
    */

    // =============================================================
    // 3. UROKO Scintillator (プラスチックシンチレータ)
    // =============================================================
    G4Polyhedra *SL_Scinti_Uroko = new G4Polyhedra("SL_Scinti_Uroko", 0 * degree, 360 * degree, 6, 2, z1_Uroko, rI1_Uroko, rO1_Uroko);
    G4LogicalVolume *LV_Scinti_Uroko = new G4LogicalVolume(SL_Scinti_Uroko, BC408, "LV_Scinti_Uroko", 0, 0, 0);

    auto rotation_2_Uroko = G4RotationMatrix(0., 0., 0.);
    auto position_2_Uroko = G4ThreeVector(0, 0, hexagon_length_Uroko - flight_length_Uroko);
    auto trans3D_2_Uroko = G4Transform3D(rotation_2_Uroko, position_2_Uroko);
    //G4VPhysicalVolume *PV_Scinti_Uroko = new G4PVPlacement(trans3D_2_Uroko, LV_Scinti_Uroko, "PV_Scinti_Uroko", LV_Mother_Uroko, false, Scinti_ID_Uroko, checkOverlaps);
    assembly_Uroko->AddPlacedVolume(LV_Scinti_Uroko, trans3D_2_Uroko);

    // =============================================================
    // 4. UROKO Light Guide (ライトガイド)
    // =============================================================
    G4Trd *SL_Guide1_Uroko = new G4Trd("SL_Guide1_Uroko", hexagon_r_Uroko, PMT_W2_Uroko / 2, (hexagon_r_Uroko * std::sqrt(3)) / 2, PMT_W_Uroko / 2, guide_L_Uroko / 2);
    G4Polyhedra *SL_Guide2_Uroko = new G4Polyhedra("SL_Guide2_Uroko", 0 * degree, 360 * degree, 6, 2, z2_Uroko, rI1_Uroko, rO2_Uroko);
    G4IntersectionSolid *SL_Guide3_Uroko = new G4IntersectionSolid("SL_Guide3_Uroko", SL_Guide1_Uroko, SL_Guide2_Uroko);
    G4Box *SL_Guide4_Uroko = new G4Box("SL_Guide4_Uroko", PMT_W2_Uroko / 2, PMT_W_Uroko / 2, guide_S_Uroko / 2);

    auto rotation_4_Uroko = G4RotationMatrix(0., 0., 0.);
    auto position_4_Uroko = G4ThreeVector(0, 0, (guide_L_Uroko / 2) - guide_S_Uroko + (thickness_Uroko / 2));
    auto trans3D_4_Uroko = G4Transform3D(rotation_4_Uroko, position_4_Uroko);
    G4UnionSolid *SL_Guide_Uroko = new G4UnionSolid("SL_Guide_Uroko", SL_Guide3_Uroko, SL_Guide4_Uroko, trans3D_4_Uroko);
    G4LogicalVolume *LV_Guide_Uroko = new G4LogicalVolume(SL_Guide_Uroko, acrylic, "LV_Guide_Uroko", 0, 0, 0);

    auto rotation_5_Uroko = G4RotationMatrix(0., 0., 0.);
    auto position_5_Uroko = G4ThreeVector(0, 0, hexagon_length_Uroko - flight_length_Uroko + (guide_L_Uroko / 2) + thickness_Uroko);
    auto trans3D_5_Uroko = G4Transform3D(rotation_5_Uroko, position_5_Uroko);
    //G4VPhysicalVolume *PV_Guide_Uroko = new G4PVPlacement(trans3D_5_Uroko, LV_Guide_Uroko, "PV_Guide_Uroko", LV_Mother_Uroko, false, Guide_ID_Uroko, checkOverlaps);
    assembly_Uroko->AddPlacedVolume(LV_Guide_Uroko, trans3D_5_Uroko);

    // =============================================================
    // 5. UROKO PMT 1 & 光電面 (カソード)
    // =============================================================
    G4Box *SL_PMT_Uroko = new G4Box("SL_PMT_Uroko", PMT_W_Uroko / 2, PMT_W_Uroko / 2, PMT_L_Uroko / 2);
    G4LogicalVolume *LV_PMT_Uroko = new G4LogicalVolume(SL_PMT_Uroko, glass, "LV_PMT_Uroko", 0, 0, 0);

    auto rotation_6_Uroko = G4RotationMatrix(0., 0., 0.);
    auto position_6_Uroko = G4ThreeVector(-PMT_C_Uroko / 2, 0, hexagon_length_Uroko - flight_length_Uroko + thickness_Uroko - 1.0 + guide_L_Uroko + guide_S_Uroko + (PMT_L_Uroko / 2));
    auto trans3D_6_Uroko = G4Transform3D(rotation_6_Uroko, position_6_Uroko);
    //G4VPhysicalVolume *PV_PMT_Uroko = new G4PVPlacement(trans3D_6_Uroko, LV_PMT_Uroko, "PV_PMT_Uroko", LV_Mother_Uroko, false, PMT_ID_Uroko, checkOverlaps);
    assembly_Uroko->AddPlacedVolume(LV_PMT_Uroko, trans3D_6_Uroko);

    G4Box *SL_PMT_Cathode_Uroko = new G4Box("SL_PMT_Cathode_Uroko", cathode_W_Uroko / 2, cathode_W_Uroko / 2, cathode_T_Uroko / 2);
    G4LogicalVolume *LV_PMT_Cathode_Uroko = new G4LogicalVolume(SL_PMT_Cathode_Uroko, aluminum, "LV_PMT_Cathode_Uroko", 0, 0, 0);

    //cathodeはPMT内に配置されているので、PVPでOK
    auto rotation_7_Uroko = G4RotationMatrix(0., 0., 0.);
    auto position_7_Uroko = G4ThreeVector(0, 0, -(PMT_L_Uroko / 2) + 3 * (cathode_T_Uroko / 2));
    auto trans3D_7_Uroko = G4Transform3D(rotation_7_Uroko, position_7_Uroko);
    G4VPhysicalVolume *PV_PMT_Cathode_Uroko = new G4PVPlacement(trans3D_7_Uroko, LV_PMT_Cathode_Uroko, "PV_PMT_Cathode_Uroko", LV_PMT_Uroko, false, Cathode_ID_Uroko, checkOverlaps);


    // =============================================================
    // 6. UROKO PMT 2 & 光電面 (カソード)
    // =============================================================
    G4Box *SL_PMT2_Uroko = new G4Box("SL_PMT2_Uroko", PMT_W_Uroko / 2, PMT_W_Uroko / 2, PMT_L_Uroko / 2);
    G4LogicalVolume *LV_PMT2_Uroko = new G4LogicalVolume(SL_PMT2_Uroko, glass, "LV_PMT2_Uroko", 0, 0, 0);

    auto rotation_8_Uroko = G4RotationMatrix(0., 0., 0.);
    auto position_8_Uroko = G4ThreeVector(PMT_C_Uroko / 2, 0, hexagon_length_Uroko - flight_length_Uroko + thickness_Uroko - 1.0 + guide_L_Uroko + guide_S_Uroko + (PMT_L_Uroko / 2));
    auto trans3D_8_Uroko = G4Transform3D(rotation_8_Uroko, position_8_Uroko);
    //G4VPhysicalVolume *PV_PMT2_Uroko = new G4PVPlacement(trans3D_8_Uroko, LV_PMT2_Uroko, "PV_PMT2_Uroko", LV_Mother_Uroko, false, PMT2_ID_Uroko, checkOverlaps);
    assembly_Uroko->AddPlacedVolume(LV_PMT2_Uroko, trans3D_8_Uroko);

    G4Box *SL_PMT_Cathode2_Uroko = new G4Box("SL_PMT_Cathode2_Uroko", cathode_W_Uroko / 2, cathode_W_Uroko / 2, cathode_T_Uroko / 2);
    G4LogicalVolume *LV_PMT_Cathode2_Uroko = new G4LogicalVolume(SL_PMT_Cathode2_Uroko, aluminum, "LV_PMT_Cathode2_Uroko", 0, 0, 0);

    auto rotation_9_Uroko = G4RotationMatrix(0., 0., 0.);
    auto position_9_Uroko = G4ThreeVector(0, 0, -(PMT_L_Uroko / 2) + 3 * (cathode_T_Uroko / 2));
    auto trans3D_9_Uroko = G4Transform3D(rotation_9_Uroko, position_9_Uroko);
    G4VPhysicalVolume *PV_PMT_Cathode2_Uroko = new G4PVPlacement(trans3D_9_Uroko, LV_PMT_Cathode2_Uroko, "PV_PMT_Cathode2_Uroko", LV_PMT2_Uroko, false, Cathode2_ID_Uroko, checkOverlaps);
        
    // =============================================================
    // 可視化属性の設定 (UROKO)
    // =============================================================

    /*
    //シンチレータ前の空気層は見えないように設定
    G4VisAttributes* visInvisible = new G4VisAttributes();
    visInvisible->SetVisibility(false); // 非表示をONにする
    LV_Mother2_Uroko->SetVisAttributes(visInvisible); // シンチレータ手前の空気層を非表示
    */

    // シンチレータ部分：青 (透明度 0.5)
    G4VisAttributes* visScinti_Uroko = new G4VisAttributes(G4Color(0.0, 0.0, 1.0, 0.5));
    visScinti_Uroko->SetForceSolid(true); // Wireframeではなく不透明な立体として表示
    LV_Scinti_Uroko->SetVisAttributes(visScinti_Uroko);

    // ライトガイド部分：灰色 (透明度 0.4)
    G4VisAttributes* visGuide_Uroko = new G4VisAttributes(G4Color(0.5, 0.5, 0.5, 0.4));
    visGuide_Uroko->SetForceSolid(true);
    LV_Guide_Uroko->SetVisAttributes(visGuide_Uroko);

    // PMT部分：紫 (透明度 0.8)
    G4VisAttributes* visPMT_Uroko = new G4VisAttributes(G4Color(0.5, 0.0, 0.5, 0.8));
    visPMT_Uroko->SetForceSolid(true);
    LV_PMT_Uroko->SetVisAttributes(visPMT_Uroko);
    LV_PMT2_Uroko->SetVisAttributes(visPMT_Uroko); // PMT2にも同じ色を適用


    //UROKOを複数配置
    const G4int numUrokoDetectors = 1;
    G4double updownDeg = 5.7 * deg;
    // 検出器の中心をどこに置くか（シンチレータ同士が密着する座標を計算して入れる）
    G4ThreeVector pos_Uroko[numUrokoDetectors] = {
        G4ThreeVector(1000*mm, hexagon_rr_Uroko + hexagon_rr_Uroko*(1-cos(updownDeg)), 0),
    };
    G4RotationMatrix* rot_Uroko[numUrokoDetectors];
    for (int i = 0; i < numUrokoDetectors; i++) {
        rot_Uroko[i] = new G4RotationMatrix();
        rot_Uroko[i]->rotateY(90.0 * deg);
        rot_Uroko[i]->rotateZ(updownDeg);
   
    }

    // ループで配置
    for (G4int i = 0; i < numUrokoDetectors; i++) {
        // Assemblyの配置には MakeImprint を使います
        assembly_Uroko->MakeImprint(
            LV_World,             // 配置先（ワールド空間）
            pos_Uroko[i],         // 位置
            rot_Uroko[i],         // 回転
            i,                    // コピー番号（Assembly ID）
            checkOverlaps         // 重なりチェック
        );
    }




    return PV_World;
}

void DetectorConstruction::ConstructSDandField()
{
    SensitiveDetector *sensDet = new SensitiveDetector("SensitiveDetector");
    fLV_Lig->SetSensitiveDetector(sensDet);
    G4SDManager::GetSDMpointer()->AddNewDetector(sensDet);
}


    