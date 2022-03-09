#pragma once
#include <Core/Widget.h>
#include <Widgets/Label.h>

namespace ui
{
	class SnapshotWidget;
	class SnapshotWidget;
	using SnapshotWidgetPtr = std::shared_ptr<SnapshotWidget>;
	using SnapshotWidgetRef = SnapshotWidget*;

	class SnapshotWidget : public CoreUI::Widget
	{
	public:
		DECLARE_EVENT_CLASS_NAME(SnapshotWidget)

		using ButtonImages = std::array<CoreUI::ImageRef, 3>;

		virtual ~SnapshotWidget() = default;
		SnapshotWidget(const SnapshotWidget&) = delete;
		SnapshotWidget& operator=(const SnapshotWidget&) = delete;
		SnapshotWidget(SnapshotWidget&&) = delete;
		SnapshotWidget& operator=(SnapshotWidget&&) = delete;

		// Images for the buttons (in order): [Load, Edit, Delete]
		void Init(ButtonImages buttonImages);

		static SnapshotWidgetPtr Create(const char* id, CoreUI::RendererRef renderer, CoreUI::Rect rect, CoreUI::CreationFlags flags = 0);

		bool HandleEvent(SDL_Event*) override;
		CoreUI::HitResult HitTest(const CoreUI::PointRef) override;

		void Draw() override;

		std::string GetText() const { return m_label->GetText(); }
		void SetText(const char* text) override { m_label->SetText(text); }

	protected:
		SnapshotWidget(const char* id, CoreUI::RendererRef renderer, CoreUI::Rect rect, CoreUI::CreationFlags flags = 0);

		CoreUI::LabelPtr m_label;
		void CreateLabel();

		CoreUI::ButtonPtr m_loadButton;
		CoreUI::ButtonPtr m_editButton;
		CoreUI::ButtonPtr m_deleteButton;
		void CreateButtons(ButtonImages buttonImages);

		struct shared_enabler;
	};
}
