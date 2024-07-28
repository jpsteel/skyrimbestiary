import gfx.io.GameDelegate;
import gfx.managers.FocusHandler;

class BestiaryWidget extends MovieClip
{
	#include "../version.as"
	
	var ShowClip:MovieClip;
	var HideClip:MovieClip;
	var savedText;

	function BestiaryWidget()
	{
		super();
		FocusHandler.instance.setFocus(this,0);
		
		this._visible = true;
		this.gotoAndStop("widgetHidden");
	}
	
	function getWidth()
	{
		return this._width;
	}
	function getHeight()
	{
		return this._height;
	}
	function setVisible(a_visible)
	{
		this._visible = a_visible;
	}
	function ShowNotification(a_show)
	{
		if (a_show == true)
		{
			GameDelegate.call("PlaySound",["UIMenuBladeOpenSD"]);
			this.gotoAndPlay("widgetShow");
		}
		else
		{
			GameDelegate.call("PlaySound",["UIMenuBladeCloseSD"]);
			this.gotoAndPlay("widgetHide");
		}
	}
	function getLabelText(a_val)
	{
		savedText = a_val.toUpperCase();
	}
	function setPosition(a_x, a_y)
	{
		this._x = a_x;
		this._y = a_y;
	}
	function setScale(a_scale:Number)
	{
		this._width = this._width * a_scale;
		this._height = this._height * a_scale;
	}
}