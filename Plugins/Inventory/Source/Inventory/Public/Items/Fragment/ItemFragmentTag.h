#pragma once

#include "NativeGameplayTags.h"

namespace FragmentTag
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GridFragment)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(ImageFragment)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(StackableFragment)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(ConsumableFragment)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(EquipableFragment)

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(ItemNameFragment)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(PrimaryStatFragment)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(FlavorTextFragment)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(ItemTypeFragment)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(SellValueFragment)

	namespace StatMod
	{
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(StatMod1);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(StatMod2);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(StatMod3);
	}
}
