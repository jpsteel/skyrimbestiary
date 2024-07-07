import gfx.controls.RadioButton;
import Shared.ButtonTextArtHolder;
import gfx.controls.ButtonGroup;
import gfx.io.GameDelegate;
import gfx.ui.InputDetails;
import Shared.GlobalFunc;
import gfx.ui.NavigationCode;
import gfx.managers.FocusHandler;
import gfx.utils.Delegate;
import flash.geom.ColorTransform;

import skyui.components.ButtonPanel;

import JSON;


class BestiaryMenu extends MovieClip
{
	#include "../version.as"

	public static var MAIN_STATE: Number = 0;

	var BestiaryMenu_mc:MovieClip;
	var CategoryList:MovieClip;
	var CategoryList_mc:MovieClip;
	var CreatureList:MovieClip;
	var CreatureList_mc:MovieClip;
	var KillcountWidget:MovieClip;
	var KillcountWidget_mc:MovieClip;
	var SummoncountWidget:MovieClip;
	var SummoncountWidget_mc:MovieClip;
	var TransformcountWidget:MovieClip;
	var TransformcountWidget_mc:MovieClip;
	var MainBackground_mc:MovieClip;
	var ImageBackground_mc:MovieClip;
	var LootBackground:MovieClip;
	var LootBackground_mc:MovieClip;
	var Divider:MovieClip;
	var TextBox_mc:MovieClip;
	var LootTitle_mc:MovieClip;
	var DebugText_mc:MovieClip;
	var ImageHolder:MovieClip;
	var ImageHolder_mc:MovieClip;
	var ResistancePanel:MovieClip;
	var ResistancePanel_mc:MovieClip;
	var WeaknessPanel:MovieClip;
	var WeaknessPanel_mc:MovieClip;
	var CreatureName_mc:MovieClip;
	var CreatureDescription_mc:MovieClip;
	var VariantSelector_mc:MovieClip;
	var creatureDescriptionTextField;
	var creatureNameTextField;
	var lootTextField;
	var NextButton;
	var PrevButton;
	var debugTextField;

	var categoriesToLoad = new Array();

	var allCreatures = new Object();
	var creatureVariants = new Array();
	var creatureKills = new Array();
	var creatureSummons = new Array();
	var creatureTransforms = new Array();

	public var previousTabButton:MovieClip;
	public var nextTabButton:MovieClip;

	var aCurrentRoaster = new Array();
	var iCurrentState:Number;
	var iVariants:Number;
	var iVariantIndex:Number;
	var bUpdated:Boolean;
	var sCurrentVariantCategory:String;
	var sCurrentCategory:String;
	var sCurrentCreature:Object;
	var sCurrentVariant:Object;
	
	var menuShown = false;
	var menuReady = false;
	var bMuteScroll = true;

	var menuHotkey:Number;
	var usingGamepad:Boolean;
	var resistanceConfig:String;

	private var _saveDisabledList:Array;
	private var _platform:Number;

	private var _deleteControls:Object;
	private var _defaultControls:Object;
	private var _kinectControls:Object;
	private var _acceptControls:Object;
	private var _cancelControls:Object;
	private var _acceptButton:MovieClip;
	private var _cancelButton:MovieClip;
	private var _currentFocus:MovieClip;

	function BestiaryMenu()
	{
		super();
		FocusHandler.instance.setFocus(this,0);

		CategoryList = CategoryList_mc;
		CreatureList = CreatureList_mc;
		KillcountWidget = KillcountWidget_mc;
		SummoncountWidget = SummoncountWidget_mc;
		TransformcountWidget = TransformcountWidget_mc;
		ImageHolder = ImageHolder_mc;
		LootBackground = LootBackground_mc;
		ResistancePanel = ResistancePanel_mc;
		WeaknessPanel = WeaknessPanel_mc;
		NextButton = VariantSelector_mc.nextBtn;
		PrevButton = VariantSelector_mc.prevBtn;
		debugTextField = DebugText_mc.debugTextField;
		creatureNameTextField = CreatureName_mc.textField;
		lootTextField = LootTitle_mc.textField;
		creatureDescriptionTextField = CreatureDescription_mc.textField;

		creatureNameTextField.text = "";
		creatureDescriptionTextField.text = "";
		lootTextField.text = "$LOOT"
		LootBackground.textField0.text = "";
		LootBackground.textField1.text = "";
		LootBackground.textField2.text = "";
		LootBackground.textField3.text = "";
		LootBackground.textField4.text = "";
		LootBackground.textField5.text = "";
		LootBackground.textField6.text = "";
		LootBackground.textField7.text = "";
		
		LootBackground.textField0.textAutoSize = "shrink";
		LootBackground.textField1.textAutoSize = "shrink";
		LootBackground.textField2.textAutoSize = "shrink";
		LootBackground.textField3.textAutoSize = "shrink";
		LootBackground.textField4.textAutoSize = "shrink";
		LootBackground.textField5.textAutoSize = "shrink";
		LootBackground.textField6.textAutoSize = "shrink";
		LootBackground.textField7.textAutoSize = "shrink";
		
		LootBackground.Item4._visible = false;
		LootBackground.Item5._visible = false;
		LootBackground.Item6._visible = false;
		LootBackground.Item7._visible = false;
		ResistancePanel.textField.text = "$RESISTANCE";
		WeaknessPanel.textField.text = "$WEAKNESS";
		ResistancePanel.AttrCheck._visible = false;
		ResistancePanel.Attr1._visible = false;
		ResistancePanel.Attr2._visible = false;
		ResistancePanel.Attr3._visible = false;
		ResistancePanel.Attr4._visible = false;
		ResistancePanel.Attr5._visible = false;
		ResistancePanel.Attr6._visible = false;
		WeaknessPanel.AttrCheck._visible = false;	
		WeaknessPanel.Attr1._visible = false;	
		WeaknessPanel.Attr2._visible = false;
		WeaknessPanel.Attr3._visible = false;
		WeaknessPanel.Attr4._visible = false;
		WeaknessPanel.Attr5._visible = false;
		WeaknessPanel.Attr6._visible = false;
		
		KillcountWidget.textKillCount.text = "";
		SummoncountWidget.textSummonCount.text = "";
		TransformcountWidget.textTransformCount.text = "";
		SummoncountWidget._visible = false;
		TransformcountWidget._visible = false;

		bUpdated = false;
		_currentFocus = CategoryList;
		_platform = 0;

		NextButton.onMouseDown = function()
		{
			if (Mouse.getTopMostEntity() == this)
			{
				this._parent._parent.onVariantButtonClick("next");
			}
		};
		PrevButton.onMouseDown = function()
		{
			if (Mouse.getTopMostEntity() == this)
			{
				this._parent._parent.onVariantButtonClick("prev");
			}
		};
		this.startPage();
	}

	function onLoad():Void
	{
		//Debug
		CategoryList.addEventListener("debugLog",this,"debugLogHandler");
		//

		CategoryList.InvalidateData();
		CreatureList.InvalidateData();

		//Categories
		CategoryList.addEventListener("itemPress",this,"onCategoryButtonPress");
		CategoryList.addEventListener("listPress",this,"onCategoryListPress");
		CategoryList.addEventListener("listMovedUp",this,"onCategoryListMoveUp");
		CategoryList.addEventListener("listMovedDown",this,"onCategoryListMoveDown");
		CategoryList.addEventListener("selectionChange",this,"onCategoryListMouseSelectionChange");
		CategoryList.disableInput = true;// Bugfix for vanilla

		//Creatures
		CreatureList.addEventListener("itemPress",this,"onCreatureButtonPress");
		CategoryList.addEventListener("listPress",this,"onCategoryListPress");
		CreatureList.addEventListener("listMovedUp",this,"onCategoryListMoveUp");
		CreatureList.addEventListener("listMovedDown",this,"onCategoryListMoveDown");
		CreatureList.addEventListener("selectionChange",this,"onCategoryListMouseSelectionChange");
		CreatureList.disableInput = true;// Bugfix for vanilla
		
		SetPlatform(_platform, false);

		VariantSelector_mc.textFieldRight.textAutoSize = "shrink";
		VariantSelector_mc.textFieldLeft.textAutoSize = "shrink";

		skyui.util.Tween.LinearTween(this,"_alpha",this._alpha,100,0.3);
		this.menuShown = true;
	}

	function startPage():Void
	{
		CategoryList.disableInput = false;// Bugfix for vanilla
		CreatureList.disableInput = false;// Bugfix for vanilla

		VariantSelector_mc._visible = false;

		if (!bUpdated)
		{
			iCurrentState = BestiaryMenu.MAIN_STATE;

			CategoryList.entryList.sortOn("text");
			CategoryList.UpdateList();
			CreatureList.entryList.sortOn("text");
			CreatureList.UpdateList();

			bUpdated = true;
			return;
		}
		//UpdateStateFocus(iCurrentState)  
	}

	function handleInput(details:InputDetails, pathToFocus:Array):Boolean
	{
		var bHandledInput: Boolean = false;
		if (GlobalFunc.IsKeyPressed(details) && this.menuShown)
		{
			if (details.navEquivalent == NavigationCode.RIGHT) {
					details.navEquivalent = NavigationCode.ENTER;
			}
			
			if (details.navEquivalent == gfx.ui.NavigationCode.GAMEPAD_B || details.navEquivalent == gfx.ui.NavigationCode.TAB || details.navEquivalent == gfx.ui.NavigationCode.GAMEPAD_BACK || details.skseKeycode == menuHotkey)
			{
				this.CloseMenu();
			}
			else if (details.navEquivalent == NavigationCode.ENTER && _currentFocus == CategoryList)
			{
				_currentFocus.onItemPress();
				_currentFocus = CreatureList;
				Divider.gotoAndStop("Left");
				bHandledInput = true;
			}
			else if (details.navEquivalent == NavigationCode.ENTER && _currentFocus == CreatureList)
			{
				_currentFocus.onItemPress();
				bHandledInput = true;
			}
			else if (details.navEquivalent == NavigationCode.UP)
			{
				_currentFocus.moveSelectionUp();
				bHandledInput = true;
			}
			else if (details.navEquivalent == NavigationCode.DOWN)
			{
				_currentFocus.moveSelectionDown();
				bHandledInput = true;
			}
			else if (details.navEquivalent == NavigationCode.LEFT && _currentFocus == CreatureList)
			{
				_currentFocus = CategoryList;
				GameDelegate.call("PlaySound",["UIMenuFocus"]);
				Divider.gotoAndStop("Right");
				bHandledInput = true;
			}
			else if (details.navEquivalent == NavigationCode.LEFT && _currentFocus == CategoryList)
			{
				GameDelegate.call("PlaySound",["UIMenuFocus"]);
				bHandledInput = true;
			}
			else if ((details.navEquivalent == NavigationCode.GAMEPAD_L1 || details.skseKeycode == 16) && VariantSelector_mc._visible == true)
			{
				onVariantButtonClick("prev")
				bHandledInput = true;
			}
			else if ((details.navEquivalent == NavigationCode.GAMEPAD_R1 || details.skseKeycode == 19) && VariantSelector_mc._visible == true)
			{
				onVariantButtonClick("next")
				bHandledInput = true;
			}
		}
		return bHandledInput;
	}

	function onCategoryButtonPress(event:Object):Void
	{
		if (event.entry.disabled)
		{
			GameDelegate.call("PlaySound",["UIMenuCancel"]);
			return;
		}

		if (iCurrentState == BestiaryMenu.MAIN_STATE)
		{
			if (sCurrentCategory != event.entry.id)
			{
				sCurrentCategory = event.entry.id;
				UpdateCreatureList(event.entry.id);
			}else{
				GameDelegate.call("PlaySound",["UIMenuFocus"]);
			}
		}
	}

	function onCreatureButtonPress(event:Object):Void
	{
		if (event.entry.disabled || event.entry.text == "EMPTY")
		{
			GameDelegate.call("PlaySound",["UIMenuCancel"]);
			return;
		}

		if (iCurrentState == BestiaryMenu.MAIN_STATE)
		{
			var selectionId = event.entry.id.toUpperCase();
			if (sCurrentCreature.id != selectionId){
				GameDelegate.call("PlaySound",["UIMenuOK"]);
				Divider.gotoAndStop("Left");
				for (var i in aCurrentRoaster){
					if (aCurrentRoaster[i].base == selectionId){
						iVariantIndex = 0;
						sCurrentVariantCategory = sCurrentCategory;
						HandleVariants(aCurrentRoaster[i].base, aCurrentRoaster[i].variants);
						break;
					}
				}
				sCurrentCreature = event.entry;
			}else{
				GameDelegate.call("PlaySound",["UIMenuFocus"]);
			}
		}
	}

	function UpdateCreatureList(categoryName):Void
	{
		CreatureList.entryList = [];
		aCurrentRoaster = [];
		var _tempCreaturesList = new Array();
		for (var i in allCreatures[categoryName]){
			var _tempCreature = allCreatures[categoryName][i];
			for (var j in _tempCreature.variants){
				if (_tempCreature.variants[j].name.toUpperCase() == _tempCreature.main.toUpperCase()){
					_tempCreaturesList.unshift(_tempCreature.variants[j]);
				}else{
					_tempCreaturesList.push(_tempCreature.variants[j]);
				}
			}
			if (_tempCreaturesList.length > 0){
				var _tempCreatureName = _tempCreature.main.toUpperCase();
				aCurrentRoaster.push({base:_tempCreatureName, variants:_tempCreaturesList});
				setCreatureDisplayName(categoryName, _tempCreatureName);
				_tempCreaturesList = [];
			}
		}
		
		if (aCurrentRoaster.length < 1){
			if (menuReady == true)
				GameDelegate.call("PlaySound",["UIMenuCancel"]);
			CreatureList.entryList.push({text:"EMPTY", textColor:0x818181, _alpha: 30});
			CreatureList.InvalidateData();
		}else{
			if (menuReady == true)
				GameDelegate.call("PlaySound",["UIMenuOk"]);
		}
	}

	function UpdateCreatureInfo(creatureId, variants):Void
	{
		iVariants = variants.length;
		if (iVariants > 1){
			VariantSelector_mc._visible = true;
			if (iVariantIndex == 0){
				PrevButton._alpha = 35;
				NextButton._alpha = 100;
				VariantSelector_mc.prevMappedButton._alpha = 35;
				VariantSelector_mc.nextMappedButton._alpha = 100;
				VariantSelector_mc.textFieldLeft.text = "";
				VariantSelector_mc.textFieldRight.text = creatureVariants[iVariantIndex + 1].name;
			}else if (iVariantIndex == iVariants - 1){
				PrevButton._alpha = 100;
				NextButton._alpha = 35;
				VariantSelector_mc.prevMappedButton._alpha = 100;
				VariantSelector_mc.nextMappedButton._alpha = 35;
				VariantSelector_mc.textFieldLeft.text = creatureVariants[iVariantIndex - 1].name;
				VariantSelector_mc.textFieldRight.text = "";
			}else{
				PrevButton._alpha = 100;
				NextButton._alpha = 100;
				VariantSelector_mc.prevMappedButton._alpha = 100;
				VariantSelector_mc.nextMappedButton._alpha = 100;
				VariantSelector_mc.textFieldLeft.text = creatureVariants[iVariantIndex - 1].name;
				VariantSelector_mc.textFieldRight.text = creatureVariants[iVariantIndex + 1].name;
			}
		}else{
			VariantSelector_mc._visible = false;
		}
		
		LoadCreatureInfo(creatureId, creatureVariants[iVariantIndex].id);
		LoadCreatureImage(creatureId, creatureVariants[iVariantIndex].id);
		LoadCreatureResist(creatureId, creatureVariants[iVariantIndex].id);
		LoadCreatureLoot(creatureId, creatureVariants[iVariantIndex].id);
		SetupCounters(creatureVariants[iVariantIndex]);
		sCurrentVariant = creatureVariants[iVariantIndex];
	}
	
	function SetupCounters(creature):Void
	{
		if (creature.kills > 999){
			KillcountWidget.background.gotoAndStop("max");
			KillcountWidget.textKillCount.text = "999+";
		}else if (creature.kills > 99){
			KillcountWidget.background.gotoAndStop("large");
			KillcountWidget.textKillCount.text = creature.kills;
		}else if (creature.kills > 9){
			KillcountWidget.background.gotoAndStop("medium");
			KillcountWidget.textKillCount.text = creature.kills;
		}else{
			KillcountWidget.background.gotoAndStop("small");
			KillcountWidget.textKillCount.text = creature.kills;
		}
		
		if (creature.summons > 999){
			SummoncountWidget.background.gotoAndStop("max");
			SummoncountWidget.textSummonCount.text = "999+";
			SummoncountWidget._visible = true;
		}else if (creature.summons > 99){
			SummoncountWidget.background.gotoAndStop("large");
			SummoncountWidget.textSummonCount.text = creature.summons;
			SummoncountWidget._visible = true;
		}else if (creature.summons > 9){
			SummoncountWidget.background.gotoAndStop("medium");
			SummoncountWidget.textSummonCount.text = creature.summons;
			SummoncountWidget._visible = true;
		}else if (creature.summons > 0){
			SummoncountWidget.background.gotoAndStop("small");
			SummoncountWidget.textSummonCount.text = creature.summons;
			SummoncountWidget._visible = true;
		}else{
			SummoncountWidget._visible = false;
		}
		
		if (creature.transforms > 999){
			TransformcountWidget.background.gotoAndStop("max");
			TransformcountWidget.textTransformCount.text = "999+";
			TransformcountWidget._visible = true;
		}else if (creature.transforms > 99){
			TransformcountWidget.background.gotoAndStop("large");
			TransformcountWidget.textTransformCount.text = creature.transforms;
			TransformcountWidget._visible = true;
		}else if (creature.transforms > 9){
			TransformcountWidget.background.gotoAndStop("medium");
			TransformcountWidget.textTransformCount.text = creature.transforms;
			TransformcountWidget._visible = true;
		}else if (creature.transforms > 0){
			TransformcountWidget.background.gotoAndStop("small");
			TransformcountWidget.textTransformCount.text = creature.transforms;
			TransformcountWidget._visible = true;
		}else{
			TransformcountWidget._visible = false;
		}
	}

	function CloseMenu():Void
	{
		SendLastEntry();
		skse.SendModEvent("BestiaryMenu","CloseMenuNoChoice");
		var _loc2_ = mx.utils.Delegate.create(this, function ()
		{
			skse.CloseMenu("CustomMenu");
		});
		skyui.util.Tween.LinearTween(this,"_alpha",this._alpha,0,0.3,_loc2_());
		GameDelegate.call("PlaySound",["UIJournalClose"]);
	}

	function DoHideMenu():Void
	{
		_parent.gotoAndPlay("fadeOut");
	}

	function DoShowMenu():Void
	{
		_parent.gotoAndPlay("fadeIn");
	}

	function RefreshSystemButtons()
	{
		_saveDisabledList.push(true);
		GameDelegate.call("SetSaveDisabled",_saveDisabledList);
		_saveDisabledList.pop();

		CategoryList.UpdateList();
	}

	function onVariantButtonClick(buttonId:String)
	{
		if (buttonId == "next" && iVariantIndex + 1 < iVariants)
		{
			GameDelegate.call("PlaySound",["UIMenuPrevNext"]);
			if (iVariantIndex + 2 == iVariants)
			{
				NextButton._alpha = 35;
				PrevButton._alpha = 100;
				VariantSelector_mc.nextMappedButton._alpha = 35;
				VariantSelector_mc.prevMappedButton._alpha = 100;
				VariantSelector_mc.textFieldRight.text = "";
			}
			else
			{
				NextButton._alpha = 100;
				PrevButton._alpha = 100;
				VariantSelector_mc.nextMappedButton._alpha = 100;
				VariantSelector_mc.prevMappedButton._alpha = 100;
				VariantSelector_mc.textFieldRight.text = creatureVariants[iVariantIndex + 2].name;
			}
			VariantSelector_mc.textFieldLeft.text = sCurrentVariant.name;
			sCurrentVariant = creatureVariants[iVariantIndex + 1];
			creatureNameTextField.text = sCurrentVariant.name;
			creatureDescriptionTextField.text = sCurrentVariant.description;
			LoadCreatureImage(sCurrentCreature.id, sCurrentVariant.id, sCurrentVariantCategory);
			LoadCreatureLoot(sCurrentCreature.id, sCurrentVariant.id, sCurrentVariantCategory);
			LoadCreatureResist(sCurrentCreature.id, sCurrentVariant.id, sCurrentVariantCategory);
			SetupCounters(sCurrentVariant);
			iVariantIndex += 1;
		}
		else if (buttonId == "prev" && iVariantIndex > 0)
		{
			GameDelegate.call("PlaySound",["UIMenuPrevNext"]);
			if (iVariantIndex - 1 == 0)
			{
				PrevButton._alpha = 35;
				NextButton._alpha = 100;
				VariantSelector_mc.prevMappedButton._alpha = 35;
				VariantSelector_mc.nextMappedButton._alpha = 100;
				VariantSelector_mc.textFieldLeft.text = "";
			}
			else
			{
				PrevButton._alpha = 100;
				NextButton._alpha = 100;
				VariantSelector_mc.nextMappedButton._alpha = 100;
				VariantSelector_mc.prevMappedButton._alpha = 100;
				VariantSelector_mc.textFieldLeft.text = creatureVariants[iVariantIndex - 2].name;
			}
			VariantSelector_mc.textFieldRight.text = sCurrentVariant.name;
			sCurrentVariant = creatureVariants[iVariantIndex - 1];
			creatureNameTextField.text = sCurrentVariant.name;
			creatureDescriptionTextField.text = sCurrentVariant.description;
			LoadCreatureImage(sCurrentCreature.id, sCurrentVariant.id, sCurrentVariantCategory);
			LoadCreatureLoot(sCurrentCreature.id, sCurrentVariant.id, sCurrentVariantCategory);
			LoadCreatureResist(sCurrentCreature.id, sCurrentVariant.id, sCurrentVariantCategory);
			SetupCounters(sCurrentVariant);
			iVariantIndex -= 1;
		}
	}

	function onCategoryListMoveUp(event:Object):Void
	{
		GameDelegate.call("PlaySound",["UIMenuFocus"]);
		if (event.scrollChanged == true)
		{
			CategoryList._parent.gotoAndPlay("moveUp");
		}
	}

	function onCategoryListMoveDown(event:Object):Void
	{
		if (!bMuteScroll)
			GameDelegate.call("PlaySound",["UIMenuFocus"]);
		if (event.scrollChanged == true)
		{
			CategoryList._parent.gotoAndPlay("moveDown");
		}
	}

	function onCategoryListMouseSelectionChange(event:Object):Void
	{
		if (event.keyboardOrMouse == 0 && event.index != -1)
		{
			GameDelegate.call("PlaySound",["UIMenuFocus"]);
		}
	}

	function onCategoryListPress(event:Object):Void
	{
		CategoryList.disableSelection = false;
		CategoryList.UpdateList();
		CategoryList.disableSelection = true;
	}
	
	function setCreatureDisplayName(category, baseId):Void
	{
		var path = "creatures/" + category + "/" + baseId + "/" + baseId + ".json";
		var lv = new LoadVars();
		lv.onData = function(src:String)
		{
			var that = this["_this"];
			try
			{
				var creature:Object = JSON.parse(src);
				that.CreatureList.entryList.push({text:creature.name, id:creature.id});
			}
			catch (ex)
			{
				that.debugLog("[setCreatureDisplayName] for " + baseId + ": " + ex.name + ":" + ex.message + ":" + ex.at + ":" + ex.text);
			}
			that.CreatureList.entryList.sort(that.doABCSort);
			that.CreatureList.InvalidateData();
		};
		lv._this = this;
		lv.load(path);
	}

	function LoadCreatureImage(creatureId, variantId, category):Void
	{
		if (category == undefined)
			category = sCurrentVariantCategory;
			
		var path = "creatures/" + category + "/" + creatureId + "/" + variantId + ".dds";
		var holder_width = 480;
		var holder_height = 480;

		var imageListener:Object = new Object();
		imageListener.onLoadInit = function(target_mc:MovieClip)
		{
			target_mc._width = holder_width;
			target_mc._height = holder_height;
		}
		imageListener._this = this;
		var imageLoader:MovieClipLoader = new MovieClipLoader();
		imageLoader.addListener(imageListener);
		imageLoader.loadClip(path,ImageHolder);
	}

	function LoadCreatureInfo(creatureId, variantId, category):Void
	{
		if (category == undefined)
			category = sCurrentVariantCategory;
			
		var path = "creatures/" + category + "/" + creatureId + "/" + variantId + ".json";
		var lv = new LoadVars();
		lv.onData = function(src:String)
		{
			var that = this["_this"];
			try
			{
				var creature:Object = JSON.parse(src);
				that.creatureNameTextField.text = creature.name;
				that.creatureDescriptionTextField.text = creature.description;
			}
			catch (ex)
			{
				//that.debugLog("[LoadCreatureInfo] for " + variantId + " at " + path + ": " + ex.name + ":" + ex.message + ":" + ex.at + ":" + ex.text);
			}
		};
		lv._this = this;
		lv.load(path);
	}
	
	function LoadCreatureResist(creatureId, variantId, category):Void
	{
		if (category == undefined)
			category = sCurrentVariantCategory;
			
		var path = "creatures/" + category + "/" + creatureId + "/" + variantId + "_RESIST.json";
		var lv = new LoadVars();
		lv.onData = function(src:String)
		{
			var that = this["_this"];
			try
			{
				var creature:Object = JSON.parse(src);
				that.DisplayResistAndWeak(creature.resistance[that.resistanceConfig], "resistance");
				that.DisplayResistAndWeak(creature.weakness[that.resistanceConfig], "weakness");
			}
			catch (ex)
			{
				//that.debugLog("[LoadCreatureInfo] for " + variantId + " at " + path + ": " + ex.name + ":" + ex.message + ":" + ex.at + ":" + ex.text);
			}
		};
		lv._this = this;
		lv.load(path);
	}
	
	function LoadCreatureLoot(creatureId, variantId, category):Void
	{
		if (category == undefined)
			category = sCurrentVariantCategory;
			
		var path = "creatures/" + category + "/" + creatureId + "/" + variantId + "_LOOT.json";
		var lv = new LoadVars();
		lv.onData = function(src:String)
		{
			var that = this["_this"];
			try
			{
				var creature:Object = JSON.parse(src);
				that.DisplayLoot(creature.loot);
			}
			catch (ex)
			{
				//that.debugLog("[LoadCreatureInfo] for " + variantId + " at " + path + ": " + ex.name + ":" + ex.message + ":" + ex.at + ":" + ex.text);
			}
		};
		lv._this = this;
		lv.load(path);
	}

	function HandleVariants(creatureId, variants)
	{
		creatureVariants = [];
		creatureKills = [];
		creatureSummons = [];
		creatureTransforms = [];
		var lv = new LoadVars();
		lv.onData = function(src:String)
		{
			var that = this["_this"];
			try
			{
				var creature:Object = JSON.parse(src);
				for (var j in that.creatureKills){
					if (that.creatureKills[j].victim.toUpperCase() == creature.id.toUpperCase()){
						creature.kills = that.creatureKills[j].count;
						break;
					}
				}
				for (var j in that.creatureSummons){
					if (that.creatureSummons[j].summon.toUpperCase() == creature.id.toUpperCase()){
						creature.summons = that.creatureSummons[j].count;
						break;
					}
				}
				for (var j in that.creatureTransforms){
					if (that.creatureTransforms[j].beast.toUpperCase() == creature.id.toUpperCase()){
						creature.transforms = that.creatureTransforms[j].count;
						break;
					}
				}
				that.creatureVariants.push(creature);
				that.UpdateCreatureInfo(creatureId, variants);
			}
			catch (ex)
			{
				that.debugLog("[LoadCreatureInfo] for " + variants[i] + " at " + path + ": " + ex.name + ":" + ex.message + ":" + ex.at + ":" + ex.text);
			}
		};
		
		for (var i in variants){
			var path = "creatures/" + sCurrentVariantCategory + "/" + creatureId + "/" + variants[variants.length - i - 1].name + ".json";
			creatureKills.push({victim:variants[variants.length - i - 1].name,count:variants[variants.length - i - 1].kills})
			creatureSummons.push({summon:variants[variants.length - i - 1].name,count:variants[variants.length - i - 1].summons})
			creatureTransforms.push({beast:variants[variants.length - i - 1].name,count:variants[variants.length - i - 1].transforms})
			lv._this = this;
			lv.load(path);
		}
	}
	
	function DisplayLoot(lootList): Void{
		if (lootList == undefined){
			lootList = [];
		}else if (lootList[0][0] == undefined){
			lootList = [];
		}else if (lootList.length > 8){
			lootList = lootList.slice(0,8);
		}
		
		switch (lootList.length) {
			case 0:
				LootBackground.LootContainer_mc._height = 123.30;
				LootBackground.LootContainer_mc._y = -8.4;
			
				LootBackground.Item4._visible = false;
				LootBackground.Item5._visible = false;
				LootBackground.Item6._visible = false;
				LootBackground.Item7._visible = false;
				LootBackground.textField4.text = "";
				LootBackground.textField5.text = "";
				LootBackground.textField6.text = "";
				LootBackground.textField7.text = "";
				
				LootBackground.textField0.text = "";
				LootBackground.Item0.gotoAndStop("empty");
				LootBackground.Item0.attachMovie("empty","icon1", this.getNextHighestDepth(), {_x:2.5,_y:2.5,_xscale:25,_yscale:25});
				LootBackground.textField1.text = "";
				LootBackground.Item1.gotoAndStop("empty");
				LootBackground.Item1.attachMovie("empty","icon2", this.getNextHighestDepth(), {_x:2.5,_y:2.5,_xscale:25,_yscale:25});
				LootBackground.textField2.text = "";
				LootBackground.Item2.gotoAndStop("empty");
				LootBackground.Item2.attachMovie("empty","icon3", this.getNextHighestDepth(), {_x:2.5,_y:2.5,_xscale:25,_yscale:25});
				LootBackground.textField3.text = "";
				LootBackground.Item3.gotoAndStop("empty");
				LootBackground.Item3.attachMovie("empty","icon4", this.getNextHighestDepth(), {_x:2.5,_y:2.5,_xscale:25,_yscale:25});
				break;
				
			case 1:
				LootBackground.LootContainer_mc._height = 123.30;
				LootBackground.LootContainer_mc._y = -8.4;
				
				LootBackground.Item4._visible = false;
				LootBackground.Item5._visible = false;
				LootBackground.Item6._visible = false;
				LootBackground.Item7._visible = false;
				LootBackground.textField4.text = "";
				LootBackground.textField5.text = "";
				LootBackground.textField6.text = "";
				LootBackground.textField7.text = "";
				
				LootBackground.textField0.text = lootList[0][0].toUpperCase();
				LootBackground.Item0.gotoAndStop("normal");
				LootBackground.Item0.attachMovie(lootList[0][1],"icon1", this.getNextHighestDepth(), {_x:2.5,_y:2.5,_xscale:25,_yscale:25});
				LootBackground.textField1.text = "";
				LootBackground.Item1.gotoAndStop("empty");
				LootBackground.Item1.attachMovie("empty","icon2", this.getNextHighestDepth(), {_x:2.5,_y:2.5,_xscale:25,_yscale:25});
				LootBackground.textField2.text = "";
				LootBackground.Item2.gotoAndStop("empty");
				LootBackground.Item2.attachMovie("empty","icon3", this.getNextHighestDepth(), {_x:2.5,_y:2.5,_xscale:25,_yscale:25});
				LootBackground.textField3.text = "";
				LootBackground.Item3.gotoAndStop("empty");
				LootBackground.Item3.attachMovie("empty","icon4", this.getNextHighestDepth(), {_x:2.5,_y:2.5,_xscale:25,_yscale:25});			
				break;
				
			case 2:
				LootBackground.LootContainer_mc._height = 123.30;
				LootBackground.LootContainer_mc._y = -8.4;
				
				LootBackground.Item4._visible = false;
				LootBackground.Item5._visible = false;
				LootBackground.Item6._visible = false;
				LootBackground.Item7._visible = false;
				LootBackground.textField4.text = "";
				LootBackground.textField5.text = "";
				LootBackground.textField6.text = "";
				LootBackground.textField7.text = "";
				
				LootBackground.textField0.text = lootList[0][0].toUpperCase();
				LootBackground.Item0.gotoAndStop("normal");
				LootBackground.Item0.attachMovie(lootList[0][1],"icon1", this.getNextHighestDepth(), {_x:2.5,_y:2.5,_xscale:25,_yscale:25});
				LootBackground.textField1.text = lootList[1][0].toUpperCase();
				LootBackground.Item1.gotoAndStop("normal");
				LootBackground.Item1.attachMovie(lootList[1][1],"icon2", this.getNextHighestDepth(), {_x:2.5,_y:2.5,_xscale:25,_yscale:25});
				LootBackground.textField2.text = "";
				LootBackground.Item2.gotoAndStop("empty");
				LootBackground.Item2.attachMovie("empty","icon3", this.getNextHighestDepth(), {_x:2.5,_y:2.5,_xscale:25,_yscale:25});
				LootBackground.textField3.text = "";
				LootBackground.Item3.gotoAndStop("empty");
				LootBackground.Item3.attachMovie("empty","icon4", this.getNextHighestDepth(), {_x:2.5,_y:2.5,_xscale:25,_yscale:25});
				break;
				
			case 3:
				LootBackground.LootContainer_mc._height = 123.30;
				LootBackground.LootContainer_mc._y = -8.4;
				
				LootBackground.Item4._visible = false;
				LootBackground.Item5._visible = false;
				LootBackground.Item6._visible = false;
				LootBackground.Item7._visible = false;
				LootBackground.textField4.text = "";
				LootBackground.textField5.text = "";
				LootBackground.textField6.text = "";
				LootBackground.textField7.text = "";
				
				LootBackground.textField0.text = lootList[0][0].toUpperCase();
				LootBackground.Item0.gotoAndStop("normal");
				LootBackground.Item0.attachMovie(lootList[0][1],"icon1", this.getNextHighestDepth(), {_x:2.5,_y:2.5,_xscale:25,_yscale:25});
				LootBackground.textField1.text = lootList[1][0].toUpperCase();
				LootBackground.Item1.gotoAndStop("normal");
				LootBackground.Item1.attachMovie(lootList[1][1],"icon2", this.getNextHighestDepth(), {_x:2.5,_y:2.5,_xscale:25,_yscale:25});
				LootBackground.textField2.text = lootList[2][0].toUpperCase();
				LootBackground.Item2.gotoAndStop("normal");
				LootBackground.Item2.attachMovie(lootList[2][1],"icon3", this.getNextHighestDepth(), {_x:2.5,_y:2.5,_xscale:25,_yscale:25});
				LootBackground.textField3.text = "";
				LootBackground.Item3.gotoAndStop("empty");
				LootBackground.Item3.attachMovie("empty","icon4", this.getNextHighestDepth(), {_x:2.5,_y:2.5,_xscale:25,_yscale:25});
				break;
			
			case 4:
				LootBackground.LootContainer_mc._height = 123.30;
				LootBackground.LootContainer_mc._y = -8.4;
				
				LootBackground.Item4._visible = false;
				LootBackground.Item5._visible = false;
				LootBackground.Item6._visible = false;
				LootBackground.Item7._visible = false;
				LootBackground.textField4.text = "";
				LootBackground.textField5.text = "";
				LootBackground.textField6.text = "";
				LootBackground.textField7.text = "";
				
				LootBackground.textField0.text = lootList[0][0].toUpperCase();
				LootBackground.Item0.gotoAndStop("normal");
				LootBackground.Item0.attachMovie(lootList[0][1],"icon1", this.getNextHighestDepth(), {_x:2.5,_y:2.5,_xscale:25,_yscale:25});
				LootBackground.textField1.text = lootList[1][0].toUpperCase();
				LootBackground.Item1.gotoAndStop("normal");
				LootBackground.Item1.attachMovie(lootList[1][1],"icon2", this.getNextHighestDepth(), {_x:2.5,_y:2.5,_xscale:25,_yscale:25});
				LootBackground.textField2.text = lootList[2][0].toUpperCase();
				LootBackground.Item2.gotoAndStop("normal");
				LootBackground.Item2.attachMovie(lootList[2][1],"icon3", this.getNextHighestDepth(), {_x:2.5,_y:2.5,_xscale:25,_yscale:25});
				LootBackground.textField3.text = lootList[3][0].toUpperCase();
				LootBackground.Item3.gotoAndStop("normal");
				LootBackground.Item3.attachMovie(lootList[3][1],"icon4", this.getNextHighestDepth(), {_x:2.5,_y:2.5,_xscale:25,_yscale:25});
				break;
				
			case 5:
				LootBackground.LootContainer_mc._height = 181.75;
				LootBackground.LootContainer_mc._y = 21.65;
				
				LootBackground.Item4._visible = true;
				LootBackground.Item5._visible = true;
				LootBackground.Item6._visible = true;
				LootBackground.Item7._visible = true;
				
				LootBackground.textField0.text = lootList[0][0].toUpperCase();
				LootBackground.Item0.gotoAndStop("normal");
				LootBackground.Item0.attachMovie(lootList[0][1],"icon1", this.getNextHighestDepth(), {_x:2.5,_y:2.5,_xscale:25,_yscale:25});
				LootBackground.textField1.text = lootList[1][0].toUpperCase();
				LootBackground.Item1.gotoAndStop("normal");
				LootBackground.Item1.attachMovie(lootList[1][1],"icon2", this.getNextHighestDepth(), {_x:2.5,_y:2.5,_xscale:25,_yscale:25});
				LootBackground.textField2.text = lootList[2][0].toUpperCase();
				LootBackground.Item2.gotoAndStop("normal");
				LootBackground.Item2.attachMovie(lootList[2][1],"icon3", this.getNextHighestDepth(), {_x:2.5,_y:2.5,_xscale:25,_yscale:25});
				LootBackground.textField3.text = lootList[3][0].toUpperCase();
				LootBackground.Item3.gotoAndStop("normal");
				LootBackground.Item3.attachMovie(lootList[3][1],"icon4", this.getNextHighestDepth(), {_x:2.5,_y:2.5,_xscale:25,_yscale:25});
				LootBackground.textField4.text = lootList[4][0].toUpperCase();
				LootBackground.Item4.gotoAndStop("normal");
				LootBackground.Item4.attachMovie(lootList[4][1],"icon5", this.getNextHighestDepth(), {_x:2.5,_y:2.5,_xscale:25,_yscale:25});
				LootBackground.textField5.text = "";
				LootBackground.Item5.gotoAndStop("empty");
				LootBackground.Item5.attachMovie("empty","icon6", this.getNextHighestDepth(), {_x:2.5,_y:2.5,_xscale:25,_yscale:25});
				LootBackground.textField6.text = "";
				LootBackground.Item6.gotoAndStop("empty");
				LootBackground.Item6.attachMovie("empty","icon7", this.getNextHighestDepth(), {_x:2.5,_y:2.5,_xscale:25,_yscale:25});
				LootBackground.textField7.text = "";
				LootBackground.Item7.gotoAndStop("empty");
				LootBackground.Item7.attachMovie("empty","icon8", this.getNextHighestDepth(), {_x:2.5,_y:2.5,_xscale:25,_yscale:25});
				break;
				
			case 6:
				LootBackground.LootContainer_mc._height = 181.75;
				LootBackground.LootContainer_mc._y = 21.65;
				
				LootBackground.Item4._visible = true;
				LootBackground.Item5._visible = true;
				LootBackground.Item6._visible = true;
				LootBackground.Item7._visible = true;
				
				LootBackground.textField0.text = lootList[0][0].toUpperCase();
				LootBackground.Item0.gotoAndStop("normal");
				LootBackground.Item0.attachMovie(lootList[0][1],"icon1", this.getNextHighestDepth(), {_x:2.5,_y:2.5,_xscale:25,_yscale:25});
				LootBackground.textField1.text = lootList[1][0].toUpperCase();
				LootBackground.Item1.gotoAndStop("normal");
				LootBackground.Item1.attachMovie(lootList[1][1],"icon2", this.getNextHighestDepth(), {_x:2.5,_y:2.5,_xscale:25,_yscale:25});
				LootBackground.textField2.text = lootList[2][0].toUpperCase();
				LootBackground.Item2.gotoAndStop("normal");
				LootBackground.Item2.attachMovie(lootList[2][1],"icon3", this.getNextHighestDepth(), {_x:2.5,_y:2.5,_xscale:25,_yscale:25});
				LootBackground.textField3.text = lootList[3][0].toUpperCase();
				LootBackground.Item3.gotoAndStop("normal");
				LootBackground.Item3.attachMovie(lootList[3][1],"icon4", this.getNextHighestDepth(), {_x:2.5,_y:2.5,_xscale:25,_yscale:25});
				LootBackground.textField4.text = lootList[4][0].toUpperCase();
				LootBackground.Item4.gotoAndStop("normal");
				LootBackground.Item4.attachMovie(lootList[4][1],"icon5", this.getNextHighestDepth(), {_x:2.5,_y:2.5,_xscale:25,_yscale:25});
				LootBackground.textField5.text = lootList[5][0].toUpperCase();
				LootBackground.Item5.gotoAndStop("normal");
				LootBackground.Item5.attachMovie(lootList[5][1],"icon6", this.getNextHighestDepth(), {_x:2.5,_y:2.5,_xscale:25,_yscale:25});
				LootBackground.textField6.text = "";
				LootBackground.Item6.gotoAndStop("empty");
				LootBackground.Item6.attachMovie("empty","icon7", this.getNextHighestDepth(), {_x:2.5,_y:2.5,_xscale:25,_yscale:25});
				LootBackground.textField7.text = "";
				LootBackground.Item7.gotoAndStop("empty");
				LootBackground.Item7.attachMovie("empty","icon8", this.getNextHighestDepth(), {_x:2.5,_y:2.5,_xscale:25,_yscale:25});
				break;
				
			case 7:
				LootBackground.LootContainer_mc._height = 181.75;
				LootBackground.LootContainer_mc._y = 21.65;
				
				LootBackground.Item4._visible = true;
				LootBackground.Item5._visible = true;
				LootBackground.Item6._visible = true;
				LootBackground.Item7._visible = true;
				
				LootBackground.textField0.text = lootList[0][0].toUpperCase();
				LootBackground.Item0.gotoAndStop("normal");
				LootBackground.Item0.attachMovie(lootList[0][1],"icon1", this.getNextHighestDepth(), {_x:2.5,_y:2.5,_xscale:25,_yscale:25});
				LootBackground.textField1.text = lootList[1][0].toUpperCase();
				LootBackground.Item1.gotoAndStop("normal");
				LootBackground.Item1.attachMovie(lootList[1][1],"icon2", this.getNextHighestDepth(), {_x:2.5,_y:2.5,_xscale:25,_yscale:25});
				LootBackground.textField2.text = lootList[2][0].toUpperCase();
				LootBackground.Item2.gotoAndStop("normal");
				LootBackground.Item2.attachMovie(lootList[2][1],"icon3", this.getNextHighestDepth(), {_x:2.5,_y:2.5,_xscale:25,_yscale:25});
				LootBackground.textField3.text = lootList[3][0].toUpperCase();
				LootBackground.Item3.gotoAndStop("normal");
				LootBackground.Item3.attachMovie(lootList[3][1],"icon4", this.getNextHighestDepth(), {_x:2.5,_y:2.5,_xscale:25,_yscale:25});
				LootBackground.textField4.text = lootList[4][0].toUpperCase();
				LootBackground.Item4.gotoAndStop("normal");
				LootBackground.Item4.attachMovie(lootList[4][1],"icon5", this.getNextHighestDepth(), {_x:2.5,_y:2.5,_xscale:25,_yscale:25});
				LootBackground.textField5.text = lootList[5][0].toUpperCase();
				LootBackground.Item5.gotoAndStop("normal");
				LootBackground.Item5.attachMovie(lootList[5][1],"icon6", this.getNextHighestDepth(), {_x:2.5,_y:2.5,_xscale:25,_yscale:25});
				LootBackground.textField6.text = lootList[6][0].toUpperCase();
				LootBackground.Item6.gotoAndStop("normal");
				LootBackground.Item6.attachMovie(lootList[6][1],"icon7", this.getNextHighestDepth(), {_x:2.5,_y:2.5,_xscale:25,_yscale:25});
				LootBackground.textField7.text = "";
				LootBackground.Item7.gotoAndStop("empty");
				LootBackground.Item7.attachMovie("empty","icon8", this.getNextHighestDepth(), {_x:2.5,_y:2.5,_xscale:25,_yscale:25});
				break;
				
			case 8:
				LootBackground.LootContainer_mc._height = 181.75;
				LootBackground.LootContainer_mc._y = 21.65;
				
				LootBackground.Item4._visible = true;
				LootBackground.Item5._visible = true;
				LootBackground.Item6._visible = true;
				LootBackground.Item7._visible = true;
				
				LootBackground.textField0.text = lootList[0][0].toUpperCase();
				LootBackground.Item0.gotoAndStop("normal");
				LootBackground.Item0.attachMovie(lootList[0][1],"icon1", this.getNextHighestDepth(), {_x:2.5,_y:2.5,_xscale:25,_yscale:25});
				LootBackground.textField1.text = lootList[1][0].toUpperCase();
				LootBackground.Item1.gotoAndStop("normal");
				LootBackground.Item1.attachMovie(lootList[1][1],"icon2", this.getNextHighestDepth(), {_x:2.5,_y:2.5,_xscale:25,_yscale:25});
				LootBackground.textField2.text = lootList[2][0].toUpperCase();
				LootBackground.Item2.gotoAndStop("normal");
				LootBackground.Item2.attachMovie(lootList[2][1],"icon3", this.getNextHighestDepth(), {_x:2.5,_y:2.5,_xscale:25,_yscale:25});
				LootBackground.textField3.text = lootList[3][0].toUpperCase();
				LootBackground.Item3.gotoAndStop("normal");
				LootBackground.Item3.attachMovie(lootList[3][1],"icon4", this.getNextHighestDepth(), {_x:2.5,_y:2.5,_xscale:25,_yscale:25});
				LootBackground.textField4.text = lootList[4][0].toUpperCase();
				LootBackground.Item4.gotoAndStop("normal");
				LootBackground.Item4.attachMovie(lootList[4][1],"icon5", this.getNextHighestDepth(), {_x:2.5,_y:2.5,_xscale:25,_yscale:25});
				LootBackground.textField5.text = lootList[5][0].toUpperCase();
				LootBackground.Item5.gotoAndStop("normal");
				LootBackground.Item5.attachMovie(lootList[5][1],"icon6", this.getNextHighestDepth(), {_x:2.5,_y:2.5,_xscale:25,_yscale:25});
				LootBackground.textField6.text = lootList[6][0].toUpperCase();
				LootBackground.Item6.gotoAndStop("normal");
				LootBackground.Item6.attachMovie(lootList[6][1],"icon7", this.getNextHighestDepth(), {_x:2.5,_y:2.5,_xscale:25,_yscale:25});
				LootBackground.textField7.text = lootList[7][0].toUpperCase();
				LootBackground.Item7.gotoAndStop("normal");
				LootBackground.Item7.attachMovie(lootList[7][1],"icon8", this.getNextHighestDepth(), {_x:2.5,_y:2.5,_xscale:25,_yscale:25});
				break;
		}
	}
	
	function DisplayResistAndWeak(attributesList, panelMode:String): Void{
		var Panel:MovieClip;
		
		if (attributesList == undefined){
			attributesList = [];
		}
		
		if (panelMode == "weakness"){
			Panel = WeaknessPanel;
		}else if(panelMode == "resistance"){
			Panel = ResistancePanel;
		}
		switch (attributesList.length) {
			case 0:
				Panel.AttrCheck._visible = true;
				Panel.Attr0._visible = true;
				Panel.Attr0.gotoAndStop("empty");
				Panel.Attr0.attachMovie("empty","icon1", this.getNextHighestDepth(), {_x:16,_y:14});
				Panel.Attr1._visible = false;
				Panel.Attr2._visible = false;
				Panel.Attr3._visible = false;
				Panel.Attr4._visible = false;
				Panel.Attr5._visible = false;
				Panel.Attr6._visible = false;
				break;
				
			case 1:
				Panel.AttrCheck._visible = false;
				Panel.Attr0._visible = true;
				if(attributesList[0].substr(0,6) == "immune"){
					Panel.Attr0.gotoAndStop("immune");
					Panel.Attr0.attachMovie(attributesList[0].substr(7),"icon1", this.getNextHighestDepth(), {_x:16,_y:14});
				}else{
					Panel.Attr0.gotoAndStop("normal");
					Panel.Attr0.attachMovie(attributesList[0],"icon1", this.getNextHighestDepth(), {_x:16,_y:14});
				}
				Panel.Attr1._visible = false;
				Panel.Attr2._visible = false;
				Panel.Attr3._visible = false;
				Panel.Attr4._visible = false;
				Panel.Attr5._visible = false;
				Panel.Attr6._visible = false;
				break;
				
			case 2:
				Panel.AttrCheck._visible = false;
				Panel.Attr0._visible = false;
				Panel.Attr1._visible = false;
				Panel.Attr2._visible = false;
				Panel.Attr3._visible = false;
				Panel.Attr4._visible = false;
				Panel.Attr5._visible = true;
				if(attributesList[0].substr(0,6) == "immune"){
					Panel.Attr5.gotoAndStop("immune");
					Panel.Attr5.attachMovie(attributesList[0].substr(7),"icon1", this.getNextHighestDepth(), {_x:16,_y:14});
				}else{
					Panel.Attr5.gotoAndStop("normal");
					Panel.Attr5.attachMovie(attributesList[0],"icon1", this.getNextHighestDepth(), {_x:16,_y:14});
				}
				Panel.Attr6._visible = true;
				if(attributesList[1].substr(0,6) == "immune"){
					Panel.Attr6.gotoAndStop("immune");
					Panel.Attr6.attachMovie(attributesList[1].substr(7),"icon1", this.getNextHighestDepth(), {_x:16,_y:14});
				}else{
					Panel.Attr6.gotoAndStop("normal");
					Panel.Attr6.attachMovie(attributesList[1],"icon1", this.getNextHighestDepth(), {_x:16,_y:14});
				}
				break;
				
			case 3:
				Panel.AttrCheck._visible = false;
				Panel.Attr0._visible = true;
				if(attributesList[0].substr(0,6) == "immune"){
					Panel.Attr0.gotoAndStop("immune");
					Panel.Attr0.attachMovie(attributesList[0].substr(7),"icon1", this.getNextHighestDepth(), {_x:16,_y:14});
				}else{
					Panel.Attr0.gotoAndStop("normal");
					Panel.Attr0.attachMovie(attributesList[0],"icon1", this.getNextHighestDepth(), {_x:16,_y:14});
				}
				Panel.Attr1._visible = true;
				if(attributesList[1].substr(0,6) == "immune"){
					Panel.Attr1.gotoAndStop("immune");
					Panel.Attr1.attachMovie(attributesList[1].substr(7),"icon1", this.getNextHighestDepth(), {_x:16,_y:14});
				}else{
					Panel.Attr1.gotoAndStop("normal");
					Panel.Attr1.attachMovie(attributesList[1],"icon1", this.getNextHighestDepth(), {_x:16,_y:14});
				}
				Panel.Attr2._visible = true;
				if(attributesList[2].substr(0,6) == "immune"){
					Panel.Attr2.gotoAndStop("immune");
					Panel.Attr2.attachMovie(attributesList[2].substr(7),"icon1", this.getNextHighestDepth(), {_x:16,_y:14});
				}else{
					Panel.Attr2.gotoAndStop("normal");
					Panel.Attr2.attachMovie(attributesList[2],"icon1", this.getNextHighestDepth(), {_x:16,_y:14});
				}
				Panel.Attr3._visible = false;
				Panel.Attr4._visible = false;
				Panel.Attr5._visible = false;
				Panel.Attr6._visible = false;
				break;
				
			case 4:
				Panel.AttrCheck._visible = false;
				Panel.Attr0._visible = true;
				if(attributesList[0].substr(0,6) == "immune"){
					Panel.Attr0.gotoAndStop("immune");
					Panel.Attr0.attachMovie(attributesList[0].substr(7),"icon1", this.getNextHighestDepth(), {_x:16,_y:16});
				}else{
					Panel.Attr0.gotoAndStop("normal");
					Panel.Attr0.attachMovie(attributesList[0],"icon1", this.getNextHighestDepth(), {_x:16,_y:14});
				}
				Panel.Attr1._visible = true;
				if(attributesList[1].substr(0,6) == "immune"){
					Panel.Attr1.gotoAndStop("immune");
					Panel.Attr1.attachMovie(attributesList[1].substr(7),"icon1", this.getNextHighestDepth(), {_x:16,_y:14});
				}else{
					Panel.Attr1.gotoAndStop("normal");
					Panel.Attr1.attachMovie(attributesList[1],"icon1", this.getNextHighestDepth(), {_x:16,_y:14});
				}
				Panel.Attr2._visible = true;
				if(attributesList[2].substr(0,6) == "immune"){
					Panel.Attr2.gotoAndStop("immune");
					Panel.Attr2.attachMovie(attributesList[2].substr(7),"icon1", this.getNextHighestDepth(), {_x:16,_y:14});
				}else{
					Panel.Attr2.gotoAndStop("normal");
					Panel.Attr2.attachMovie(attributesList[2],"icon1", this.getNextHighestDepth(), {_x:16,_y:14});
				}
				Panel.Attr3._visible = true;
				if(attributesList[3].substr(0,6) == "immune"){
					Panel.Attr3.gotoAndStop("immune");
					Panel.Attr3.attachMovie(attributesList[3].substr(7),"icon1", this.getNextHighestDepth(), {_x:16,_y:14});
				}else{
					Panel.Attr3.gotoAndStop("normal");
					Panel.Attr3.attachMovie(attributesList[3],"icon1", this.getNextHighestDepth(), {_x:16,_y:14});
				}
				Panel.Attr4._visible = false;
				Panel.Attr5._visible = false;
				Panel.Attr6._visible = false;
				break;
				
			case 5:
				Panel.AttrCheck._visible = false;
				Panel.Attr0._visible = true;
				if(attributesList[0].substr(0,6) == "immune"){
					Panel.Attr0.gotoAndStop("immune");
					Panel.Attr0.attachMovie(attributesList[0].substr(7),"icon1", this.getNextHighestDepth(), {_x:16,_y:14});
				}else{
					Panel.Attr0.gotoAndStop("normal");
					Panel.Attr0.attachMovie(attributesList[0],"icon1", this.getNextHighestDepth(), {_x:16,_y:14});
				}
				Panel.Attr1._visible = true;
				if(attributesList[1].substr(0,6) == "immune"){
					Panel.Attr1.gotoAndStop("immune");
					Panel.Attr1.attachMovie(attributesList[1].substr(7),"icon1", this.getNextHighestDepth(), {_x:16,_y:14});
				}else{
					Panel.Attr1.gotoAndStop("normal");
					Panel.Attr1.attachMovie(attributesList[1],"icon1", this.getNextHighestDepth(), {_x:16,_y:14});
				}
				Panel.Attr2._visible = true;
				if(attributesList[2].substr(0,6) == "immune"){
					Panel.Attr2.gotoAndStop("immune");
					Panel.Attr2.attachMovie(attributesList[2].substr(7),"icon1", this.getNextHighestDepth(), {_x:16,_y:14});
				}else{
					Panel.Attr2.gotoAndStop("normal");
					Panel.Attr2.attachMovie(attributesList[2],"icon1", this.getNextHighestDepth(), {_x:16,_y:14});
				}
				Panel.Attr3._visible = true;
				if(attributesList[3].substr(0,6) == "immune"){
					Panel.Attr3.gotoAndStop("immune");
					Panel.Attr3.attachMovie(attributesList[3].substr(7),"icon1", this.getNextHighestDepth(), {_x:16,_y:14});
				}else{
					Panel.Attr3.gotoAndStop("normal");
					Panel.Attr3.attachMovie(attributesList[3],"icon1", this.getNextHighestDepth(), {_x:16,_y:14});
				}
				Panel.Attr4._visible = true;
				if(attributesList[4].substr(0,6) == "immune"){
					Panel.Attr4.gotoAndStop("immune");
					Panel.Attr4.attachMovie(attributesList[4].substr(7),"icon1", this.getNextHighestDepth(), {_x:16,_y:14});
				}else{
					Panel.Attr4.gotoAndStop("normal");
					Panel.Attr4.attachMovie(attributesList[4],"icon1", this.getNextHighestDepth(), {_x:16,_y:14});
				}
				Panel.Attr5._visible = false;
				Panel.Attr6._visible = false;
				break;
		}
	}

	function SetPlatform(a_platform:Number, a_bPS3Switch:Boolean):Void
	{
		CategoryList.SetPlatform(a_platform,a_bPS3Switch);
		CreatureList.SetPlatform(a_platform,a_bPS3Switch);
		
		var isGamepad = _platform != 0;

		if (a_platform == 0)
		{
			_deleteControls = {keyCode:45};// X
			_defaultControls = {keyCode:20};// T
			_kinectControls = {keyCode:37};// K
			_acceptControls = {keyCode:28};// Enter
			_cancelControls = {keyCode:15};// Tab
		}
		else
		{
			_deleteControls = {keyCode:278};// 360_X
			_defaultControls = {keyCode:279};// 360_Y
			_kinectControls = {keyCode:275};// 360_RB
			_acceptControls = {keyCode:276};// 360_A
			_cancelControls = {keyCode:277};// 360_B
		}

		_acceptButton.addEventListener("click",this,"onAcceptMousePress");
		_cancelButton.addEventListener("click",this,"onCancelMousePress");

		_platform = a_platform;
	}

	function getHotkey(hotkey)
	{
		menuHotkey = hotkey;
	}
	
	function getGamepadStatus(gamepad)
	{
		usingGamepad = gamepad;
		
		if (!usingGamepad){
			VariantSelector_mc.nextMappedButton.gotoAndStop(19);
			VariantSelector_mc.prevMappedButton.gotoAndStop(16);
		}else{
			VariantSelector_mc.nextMappedButton.gotoAndStop(275);
			VariantSelector_mc.prevMappedButton.gotoAndStop(274);
		}
	}

	function getResistanceMod(mod)
	{
		resistanceConfig = mod.toLowerCase();
	}

	function getAllCreaturesLists(creaturesLists)
	{
		GameDelegate.call("PlaySound",["UIJournalOpen"]);
		var path = "creatures/categories.json";
		var lv = new LoadVars();
		var categoryName
		lv.onData = function(src:String)
		{
			var that = this["_this"];
			try
			{
				var i = 0;
				var list:Object = JSON.parse(src);
				for (var category in list["categories"]){
					that.categoriesToLoad.push({text:list["categories"][category], id:category.toUpperCase()});
					i+=1
				}
			}
			catch (ex)
			{
				that.debugLog("[getCategoriesList]: " + ex.name + ":" + ex.message + ":" + ex.at + ":" + ex.text);
			}
			
			var creaturesPerCategory = creaturesLists.split("&");
			var backupCategory = "";
			for (var i in creaturesPerCategory){
				for (var j in that.categoriesToLoad){
					var categoryName = creaturesPerCategory[i].split(":")[0].toUpperCase();
					var creaturesListArray = creaturesPerCategory[i].split(":")[1].split(",");
					if (that.categoriesToLoad[j].id == categoryName){
						that.CategoryList.entryList.push({text:that.categoriesToLoad[j].text.toUpperCase(), id:that.categoriesToLoad[j].id.toUpperCase()});
						if (creaturesListArray.length > 0){
							if (backupCategory == ""){
								backupCategory = categoryName;
							}
							for (var j in creaturesListArray){
								var mainCrt = creaturesListArray[j].split("_")[0].toUpperCase();
								var varCrt = new Array();
								for (var k in creaturesListArray[j].split("_")[1].split("@"))
									varCrt.unshift({name:creaturesListArray[j].split("_")[1].split("@")[k].split("#")[0].toUpperCase(),kills:parseInt(creaturesListArray[j].split("_")[1].split("@")[k].split("#")[1]),summons:parseInt(creaturesListArray[j].split("_")[1].split("@")[k].split("#")[2]),transforms:parseInt(creaturesListArray[j].split("_")[1].split("@")[k].split("#")[3])});
								creaturesListArray[j] = {main: mainCrt, variants: varCrt};
							}
							var key:String = categoryName;
							that.allCreatures[key] = new Object();
							that.allCreatures[key] = creaturesListArray;
						}
					}
				}
			}
			that.CategoryList.entryList.sort(that.doABCSort);
			that.CategoryList.InvalidateData();
			
			if (that.sCurrentVariantCategory != "" && that.sCurrentCreature.id != "UNDEFINED"){
				that.UpdateCreatureList(that.sCurrentVariantCategory);
				for (var i in that.aCurrentRoaster){
					if (that.aCurrentRoaster[i].base == that.sCurrentCreature.id.toUpperCase()){
						that.HandleVariants(that.aCurrentRoaster[i].base, that.aCurrentRoaster[i].variants);
						break;
					}
				}
				var i = 0;
				while (that.CategoryList.entryList[i].text != that.sCurrentCategory){
					that.CategoryList.moveSelectionDown();
					i++;
				}
			}else if (backupCategory != ""){
				that.UpdateCreatureList(backupCategory);
				that.sCurrentCategory = backupCategory;
				that.sCurrentVariantCategory = backupCategory;
				that.iVariantIndex = 0;
				that.HandleVariants(that.aCurrentRoaster[0].base, that.aCurrentRoaster[0].variants);
				var i = 0;
				while (that.CategoryList.entryList[i].text != that.sCurrentCategory){
					that.CategoryList.moveSelectionDown();
					i++;
				}
				that.sCurrentCreature.id = that.aCurrentRoaster[0].base
			}
			that.bMuteScroll = false;
			that.menuReady = true;
		};
		lv._this = this;
		lv.load(path);
	}
	
	function SendLastEntry():Void{
		var lastEntry
		lastEntry = sCurrentVariantCategory + "," + sCurrentCreature.name + "," + sCurrentCreature.id + "," + iVariantIndex;
		skse.SendModEvent("SaveLastEntry",lastEntry);
	}
	
	function getLastEntry(lastEntry):Void{
		var aLastEntry = lastEntry.split(",");
		sCurrentCategory = aLastEntry[0].toUpperCase();
		sCurrentVariantCategory = sCurrentCategory;
		sCurrentCreature = {text:aLastEntry[1].toUpperCase(),id:aLastEntry[2].toUpperCase()};
		iVariantIndex = parseInt(aLastEntry[3]);
	}

	function doABCSort(aObj1:Object, aObj2:Object):Number
	{
		if (aObj1.text < aObj2.text)
		{
			return -1;
		}
		if (aObj1.text > aObj2.text)
		{
			return 1;
		}
		return 0;
	}

	function debugLog(message:Object):Void
	{
		if (typeof (message) == "string")
		{
			debugTextField.text += message + "\n";
		}
		else if (typeof (message) == "object")
		{
			var logText:String = "Object Details: ";
			for (var prop in message)
			{
				logText += prop + ": " + message[prop] + "; ";
			}
			debugTextField.text += logText + "\n";
		}
	}
}