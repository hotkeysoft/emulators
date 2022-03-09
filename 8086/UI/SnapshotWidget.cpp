#include "stdafx.h"
#include <Widgets/Label.h>
#include <Widgets/Button.h>
#include "SnapshotWidget.h"

using namespace CoreUI;

namespace ui
{
	const int buttonW = 24;

	SnapshotWidget::SnapshotWidget(const char* id, RendererRef renderer, Rect rect, CreationFlags flags) :
		Widget(id, renderer, nullptr, rect, nullptr, nullptr, nullptr, flags)
	{
		if (m_renderer == nullptr)
		{
			throw std::invalid_argument("no renderer");
		}
	}

	void SnapshotWidget::Init(ButtonImages buttonImages)
	{
		Widget::Init();

		CreateButtons(buttonImages);
		CreateLabel();
	}

	SnapshotWidgetPtr SnapshotWidget::Create(const char* id, RendererRef renderer, Rect rect, CreationFlags flags)
	{
		auto ptr = std::make_shared<shared_enabler>(id, renderer, rect, flags);
		return std::static_pointer_cast<SnapshotWidget>(ptr);
	}

	void SnapshotWidget::CreateLabel()
	{	
		std::string id = MakeChildWidgetId("label");
		m_label = Label::CreateAutoSize(id.c_str(), m_renderer, m_text.c_str(), m_font);

		m_label->SetAlign(Label::TEXT_H_LEFT | Label::TEXT_V_CENTER);
		m_label->SetMargin(Dimension(5, 2));
		m_label->SetPadding(0);
		m_label->SetBorder(false);
		m_label->SetParent(this);
		m_label->Init();
	}

	void SnapshotWidget::CreateButtons(ButtonImages buttonImages)
	{
		Rect parent = GetRect(false, true);

		// Load button
		{
			Rect rect = { 0, 0, buttonW, buttonW };

			std::string id = MakeChildWidgetId("load");
			m_loadButton = Button::Create(id.c_str(), m_renderer, rect, nullptr, buttonImages[0]);
			m_loadButton->SetParent(this);
			m_loadButton->Init();
		}

		// Edit button
		{
			Rect rect = { parent.w - (2 * buttonW), 0, buttonW, buttonW };

			std::string id = MakeChildWidgetId("edit");
			m_editButton = Button::Create(id.c_str(), m_renderer, rect, nullptr, buttonImages[1]);
			m_editButton->SetParent(this);
			m_editButton->Init();
		}

		// Delete button
		{
			Rect rect = { parent.w - buttonW, 0, buttonW, buttonW };

			std::string id = MakeChildWidgetId("delete");
			m_deleteButton = Button::Create(id.c_str(), m_renderer, rect, nullptr, buttonImages[2]);
			m_deleteButton->SetParent(this);
			m_deleteButton->Init();
		}
	}

	void SnapshotWidget::Draw()
	{
		if (m_parent == nullptr)
			return;

		Rect drawRect = GetRect(false, true);

		m_loadButton->Draw();
		m_editButton->Draw();
		m_deleteButton->Draw();

		// Draw and position the label manually because 
		// we draw a 3d border around it
		Rect labelRect = drawRect;
		labelRect.x += buttonW;
		labelRect.w -= 3 * buttonW;
		Draw3dFrame(&labelRect, true);
		m_label->Draw(&labelRect.Deflate(1));
	}

	HitResult SnapshotWidget::HitTest(const PointRef pt)
	{
		Rect parent = m_parent->GetClientRect(false, true);
		if (m_rect.Offset(&parent.Origin()).PointInRect(pt))
		{
			return HitResult(HitZone::HIT_CONTROL, this);
		}

		return HitZone::HIT_NOTHING;
	}

	bool SnapshotWidget::HandleEvent(SDL_Event* e)
	{
		Point pt(e->button.x, e->button.y);
		if (m_loadButton->HitTest(&pt))
		{
			return m_loadButton->HandleEvent(e);
		}
		else if (m_editButton->HitTest(&pt))
		{
			return m_editButton->HandleEvent(e);
		}
		else if (m_deleteButton->HitTest(&pt))
		{
			return m_deleteButton->HandleEvent(e);
		}
		return false;
	}

	struct SnapshotWidget::shared_enabler : public SnapshotWidget
	{
		template <typename... Args>
		shared_enabler(Args &&... args)
			: SnapshotWidget(std::forward<Args>(args)...)
		{
		}
	};
}