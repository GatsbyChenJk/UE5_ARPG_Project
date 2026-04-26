#include "Items/Fragment/ItemFragmentTag.h"

namespace FragmentTag
{
	UE_DEFINE_GAMEPLAY_TAG(GridFragment,"FragmentTag.GridFragment")
	UE_DEFINE_GAMEPLAY_TAG(ImageFragment,"FragmentTag.ImageFragment")
	UE_DEFINE_GAMEPLAY_TAG(StackableFragment,"FragmentTag.StackableFragment")
	UE_DEFINE_GAMEPLAY_TAG(ConsumableFragment,"FragmentTag.ConsumableFragment")
	UE_DEFINE_GAMEPLAY_TAG(EquipableFragment,"FragmentTag.EquipableFragment")

	UE_DEFINE_GAMEPLAY_TAG(ItemNameFragment,"FragmentTag.ItemNameFragment")
	UE_DEFINE_GAMEPLAY_TAG(PrimaryStatFragment,"FragmentTag.PrimaryStatFragment")
	UE_DEFINE_GAMEPLAY_TAG(FlavorTextFragment,"FragmentTag.FlavorTextFragment")
	UE_DEFINE_GAMEPLAY_TAG(ItemTypeFragment,"FragmentTag.ItemTypeFragment")
	UE_DEFINE_GAMEPLAY_TAG(SellValueFragment,"FragmentTag.SellValueFragment")

	namespace StatMod
	{
		UE_DEFINE_GAMEPLAY_TAG(StatMod1,"FragmentTag.StatMod.1")
		UE_DEFINE_GAMEPLAY_TAG(StatMod2,"FragmentTag.StatMod.2")
		UE_DEFINE_GAMEPLAY_TAG(StatMod3,"FragmentTag.StatMod.3")
	}
}
