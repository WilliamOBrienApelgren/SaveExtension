// Copyright 2015-2020 Piperift. All Rights Reserved.

#include "Automatron.h"
#include "Helpers/TestGameInstance.h"
#include "SaveManager.h"


class FSaveSpec_GameInstance : public Automatron::FTestSpec
{
	GENERATE_SPEC(FSaveSpec_GameInstance, "SaveExtension",
		EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter);

	USaveManager* SaveManager = nullptr;
	USavePreset* TestPreset = nullptr;

	// Helper for some test delegates
	bool bFinishTick = false;

	FSaveSpec_GameInstance()
	{
		bReuseWorldForAllTests = false;
		bCanUsePIEWorld = false;

		DefaultWorldSettings.bShouldTick = true;
		DefaultWorldSettings.GameInstance = UTestGameInstance::StaticClass();
	}

	USavePreset* CreateTestPreset();
};

void FSaveSpec_GameInstance::Define()
{
	BeforeEach([this]() {
		SaveManager = USaveManager::Get(GetMainWorld());
		TestNotNull(TEXT("SaveManager"), SaveManager);

		TestPreset = CreateTestPreset();
		TestPreset->bStoreGameInstance = true;

		TestPreset->MultithreadedFiles = ESaveASyncMode::OnlySync;
		TestPreset->MultithreadedSerialization = ESaveASyncMode::OnlySync;
		SaveManager->SetActivePreset(TestPreset);
	});

	It("GameInstance can be saved", [this]()
	{
		auto* GI = GetMainWorld()->GetGameInstance<UTestGameInstance>();
		GI->bMyBool = true;

		SaveManager->SaveSlot(0);

		TestTrue("Saved variable didn't change with save", GI->bMyBool);
		GI->bMyBool = false;

		SaveManager->LoadSlot(0);

		TestTrue("Saved variable loaded", GI->bMyBool);
	});

	AfterEach([this]()
	{
		if (SaveManager)
		{
			bFinishTick = false;
			SaveManager->DeleteAllSlots(FOnSlotsDeleted::CreateLambda([this]()
			{
				bFinishTick = true;
			}));

			TickWorldUntil(GetMainWorld(), true, [this](float) {
				return !bFinishTick;
			});
		}
		SaveManager = nullptr;
	});
}

USavePreset* FSaveSpec_GameInstance::CreateTestPreset()
{
	USavePreset* Preset = NewObject<USavePreset>(GetMainWorld());
	return Preset;
}
