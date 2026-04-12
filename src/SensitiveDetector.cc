#include "SensitiveDetector.hh"

SensitiveDetector::SensitiveDetector(G4String name) : G4VSensitiveDetector(name)
{
    //物理的に意味のない時刻で初期化
   fHitTime = -1.0;
}

SensitiveDetector::~SensitiveDetector()
{
}

void SensitiveDetector::Initialize(G4HCofThisEvent *)
{
   fHitTime = -1.0 ;
}


void SensitiveDetector::EndOfEvent(G4HCofThisEvent *)
{
    G4AnalysisManager *analysisManager = G4AnalysisManager::Instance();

    if (fHitTime >= 0.0) {
        // 列 (Column 0) に値をセット
        analysisManager->FillNtupleDColumn(0, 0, fHitTime);
        
        // 【重要】ここで「行を確定」してNtupleに書き込む！
        analysisManager->AddNtupleRow(0);
    }

}


G4bool SensitiveDetector::ProcessHits(G4Step *aStep, G4TouchableHistory *ROhist)
{
   //1.粒子名の取得と判定
    G4String particleName = aStep->GetTrack()->GetDefinition()->GetParticleName();

    if (particleName != "neutron") {
        return false; // 中性子以外（ガンマ線や二次粒子のアルファ線など）は無視して終了
    }

    //2.プロセスを取得する前の安全対策（超重要）
    // ジオメトリの境界を跨いだだけの時などは Process が Null になることがあるため、
    // 必ずポインタが存在するかチェックする。
    const G4VProcess* process = aStep->GetPostStepPoint()->GetProcessDefinedStep();
    if (!process) {
        return false; 
    }

    // 3. プロセス名の取得と判定
    G4String processName = process->GetProcessName();

    //G4cout << "ProcessName is : " << processName << G4endl;

    // 中性子捕獲 ("nCapture") が起きたかどうかを判定
    if (processName == "nCapture") {
        
        // 反応が起きた瞬間の GlobalTime を取得
        fHitTime = aStep->GetPostStepPoint()->GetGlobalTime();

        G4cout << "ProcessName is : " << processName << G4endl;

        //確認用
        G4cout << "HitTime : " << fHitTime << G4endl;
    
        // 一度捕獲反応が起きれば中性子は消滅するため、以降の追跡を強制終了させても良いです
        aStep->GetTrack()->SetTrackStatus(fStopAndKill);
        
        return true;
    }

    return false;

}