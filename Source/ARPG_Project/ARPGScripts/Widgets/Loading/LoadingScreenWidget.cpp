#include "LoadingScreenWidget.h"

void ULoadingScreenWidget::OnOpenWidget(APlayerController* OwningController)
{
	SetVisibility(ESlateVisibility::Visible);
}

void ULoadingScreenWidget::OnCloseWidget(APlayerController* OwningController)
{
	SetVisibility(ESlateVisibility::Collapsed);
}
