#include "stdafx.h"
#include <Widgets/Label.h>
#include <Widgets/Button.h>
#include "SnapshotWidget.h"

using namespace CoreUI;

namespace ui
{
	const int buttonW = 24;

	SnapshotWidgetRef SnapshotWidget::m_editingWidget = nullptr;

	SnapshotWidget::SnapshotWidget(const char* id, RendererRef renderer, Rect rect, CreationFlags flags) :
		Widget(id, renderer, nullptr, rect, nullptr, nullptr, nullptr, flags)
	{
		if (m_renderer == nullptr)
		{
			throw std::invalid_argument("no renderer");
		}
	}

	void SnapshotWidget::Init(const SnapshotInfo& info, ButtonImages buttonImages)
	{
		Widget::Init();

		m_info = info;

		CreateButtons(buttonImages);
		CreateLabel();
	}

	SnapshotWidgetPtr SnapshotWidget::Create(const char* id, RendererRef renderer, Rect rect, CreationFlags flags)
	{
		auto ptr = std::make_shared<shared_enabler>(id, renderer, rect, flags);
		return std::static_pointer_cast<SnapshotWidget>(ptr);
	}


	void SnapshotWidget::CreateButtons(ButtonImages buttonImages)
	{
		Rect parent = GetRect(false, true);

		// Load button
		{
			Rect rect = { 0, 0, buttonW, parent.h };

			std::string id = MakeChildWidgetId("load");
			m_loadButton = Button::Create(id.c_str(), m_renderer, rect, nullptr, buttonImages[0]);
			m_loadButton->SetParent(this);
			m_loadButton->Init();
		}

		// Edit button
		{
			Rect rect = { parent.w - (2 * buttonW), 0, buttonW, parent.h };

			std::string id = MakeChildWidgetId("edit");
			m_editButton = Button::Create(id.c_str(), m_renderer, rect, nullptr, buttonImages[1]);
			m_editButton->SetParent(this);
			m_editButton->Init();
		}

		// Delete button
		{
			Rect rect = { parent.w - buttonW, 0, buttonW, parent.h };

			std::string id = MakeChildWidgetId("delete");
			m_deleteButton = Button::Create(id.c_str(), m_renderer, rect, nullptr, buttonImages[2]);
			m_deleteButton->SetParent(this);
			m_deleteButton->Init();
		}
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

	void SnapshotWidget::CreateTextBox()
	{
		Rect parent = GetRect(false, true);

		Rect rect = { buttonW, 0, parent.w - (3 * buttonW), parent.h };

		std::string id = MakeChildWidgetId("text");
		m_textBox = TextBox::CreateSingleLine(id.c_str(), m_renderer, rect, m_info.GetDescription().c_str());
		m_textBox->SetParent(this);
		m_textBox->Init();
	}

	void SnapshotWidget::BeginEdit()
	{
		if (m_editingWidget && m_editingWidget != this)
		{
			m_editingWidget->EndEdit();
		}
		m_editingWidget = this;
		CreateTextBox();
		m_editButton->SetPushed(true);
		m_textBox->SetActive();
		m_textBox->SetFocus(nullptr);
	}

	void SnapshotWidget::EndEdit()
	{
		if (m_editingWidget)
		{
			m_editingWidget->InternalEndEdit();
			m_editingWidget = nullptr;
		}
	}

	void SnapshotWidget::InternalEndEdit()
	{
		// TODO: Save info

		m_textBox = nullptr;
		m_editButton->SetPushed(false);
	}

	void SnapshotWidget::UpdateLabel()
	{
		std::ostringstream os;
		os << m_text << ": ";
		if (m_info.IsLoaded())
		{
			os << m_info.GetPC()
				<< " (" << m_info.GetVideo() << ") "
				<< m_info.GetDescription();
		}
		else
		{
			os << "Unknown";
		}
		m_label->SetText(os.str().c_str());
	}

	void SnapshotWidget::Draw()
	{
		if (m_parent == nullptr)
			return;

		Rect drawRect = GetRect(false, true);

		m_loadButton->Draw();
		m_editButton->Draw();
		m_deleteButton->Draw();

		if (m_textBox) // Edit mode
		{
			m_textBox->Draw();
		}
		else // Normal mode, draw the label
		{
			// Draw and position the label manually because 
			// we draw a 3d border around it
			Rect labelRect = drawRect;
			labelRect.x += buttonW;
			labelRect.w -= 3 * buttonW;
			Draw3dFrame(&labelRect, true);

			m_label->Draw(&labelRect.Deflate(1));
		}
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
		else if (m_textBox)
		{
			return m_textBox->HandleEvent(e);
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