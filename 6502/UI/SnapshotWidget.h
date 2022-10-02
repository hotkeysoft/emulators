#pragma once
#include <Core/Widget.h>
#include <Widgets/Label.h>
#include <Widgets/TextBox.h>
#include "SnapshotInfo.h"

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

		// buttonImages: images for the buttons (in order): [Load, Edit, Delete]
		void Init(const SnapshotInfo& info, ButtonImages buttonImages);
		const SnapshotInfo& GetInfo() const { return m_info; }

		static SnapshotWidgetPtr Create(const char* id, CoreUI::RendererRef renderer, CoreUI::Rect rect, CoreUI::CreationFlags flags = 0);

		bool HandleEvent(SDL_Event*) override;
		CoreUI::HitResult HitTest(const CoreUI::PointRef) override;

		void Draw() override;

		void SetText(const char* text) override { m_text = text ? text : ""; UpdateLabel(); }

		// Shows the text box and allow editing the snapshot's description
		void BeginEdit();
		static void EndEdit();

		void ClearFocus() override;

	protected:
		SnapshotWidget(const char* id, CoreUI::RendererRef renderer, CoreUI::Rect rect, CoreUI::CreationFlags flags = 0);

		SnapshotInfo m_info;

		CoreUI::LabelPtr m_label;
		void CreateLabel();
		void UpdateLabel();

		CoreUI::TextBoxPtr m_textBox;
		void CreateTextBox();

		CoreUI::ButtonPtr m_loadButton;
		CoreUI::ButtonPtr m_editButton;
		CoreUI::ButtonPtr m_deleteButton;
		void CreateButtons(ButtonImages buttonImages);

		// Widget currently editing
		bool IsEditing() const { return m_editingWidget == this; }
		static SnapshotWidgetRef m_editingWidget;
		void InternalEndEdit();

		struct shared_enabler;
	};
}
