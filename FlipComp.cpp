#include "FlipComp.h"

#include "SceneCore/module/MO_Module.h"

#include "SceneCore/module/MO_ModuleFactory.h"
#include "SceneCore/module/MO_ModuleVisitor.h"
#include "SceneCore/module/MO_OGLContext.h"
#include "SceneCore/module/MO_SoftContext.h"
#include "SceneCore/module/MO_PortContext.h"
#include "SceneCore/module/MO_SoftRenderServices.h"

#include "SceneCore/attribute/AT_IntAttr.h"

#include "SceneCore/celcache/CA_CelRef.h"

#include "CelCore/Cel/CEL_PixmapAreaRGBA16.h"
#include "CelCore/CelAlgo/ALGO_CelAlgo.h"
#include "CelCore/Cel/CEL_Cel.h"

#include "BaseCore/misc/tutranslator.h"

#include <cstdio>

class FLIPCOMP_INTERNAL FlipComp : public MO_Module
{
	Q_TRANSLATABLE

protected:
	AT_IntAttr* _selection;

public:

	FlipComp();

	// static void filterAttributes(MO_AbstractModuleVisitor *v, MO_Module *m);
	virtual void accept(MO_AbstractModuleVisitor& v);

	virtual const QString& keyword() const;
	virtual QString baseDisplayName() const;

	static const QString& Keyword();

	virtual void oglRender(MO_OGLContext &context);
	virtual void softRender(MO_SoftContext &context);

	AT_IntAttr* selection() const { return _selection; }

};

static AT_AttrMeta g_selection( NTR("SELECTION"), QT_TRANSLATE_NOOP( "AT_Attr", "Order Selection"));

Q_TRANSLATOR(FlipComp)

FlipComp::FlipComp()
: _selection(new AT_IntAttr(this, g_selection, true, 1, 1, 2))
{
	addInImagePort();
	addInImagePort();
	addOutImagePort();

	registerAttr(_selection);
};

void FlipComp::accept(MO_AbstractModuleVisitor& v)
{
	// Just call base class
	if (!v.performDynamicOperation(this, Keyword()))
	{
		v.visit(*this);
	}
}

const QString& FlipComp::Keyword()
{
	static const QString gKeyword = NTR("FlipComp");
	return gKeyword;
}

const QString& FlipComp::keyword() const
{
	return Keyword();
}

QString FlipComp::baseDisplayName() const
{
	return NTR("FlipComp");
}

void FlipComp::oglRender(MO_OGLContext &context)
{
	MO_Module* imageModuleA = inputModule(0);
	MO_Module* imageModuleB = inputModule(1);

	if (imageModuleA && imageModuleB)
	{
		if (_selection->value(context.frame()) == 1)
		{
			imageModuleA->oglRender(context);
			imageModuleB->oglRender(context);
		} else {
			imageModuleB->oglRender(context);
			imageModuleA->oglRender(context);
		}

		return;
	}

	// Fall to render one or the other until we have both ports connected

	if (imageModuleA)
	{
		imageModuleA->oglRender(context);
		return;
	}

	if (imageModuleB)
	{
		imageModuleB->oglRender(context);
		return;
	}
}

void FlipComp::softRender(MO_SoftContext &context)
{
	if (!enabled())
	{
		setEmptyCel(context, 0);
	}

	// Output cel pointer
	CEL_Cel* result;

	// Cel Refs for A and B
	CA_CelRef imageCelA;
	CA_CelRef imageCelB;

	if (!context.inputPortContext(0).cel() || !context.inputPortContext(1).cel())
	{
		// no input
		return;
	}

	imageCelA = context.inputPortContext(0).cel();
	imageCelB = context.inputPortContext(1).cel();

	// Make new output cel
	result = new CEL_Cel;

	// Pixmaps for A and B inputs
	CEL_PixmapAreaRGBA16::Ptr pixmapA;
	CEL_PixmapAreaRGBA16::Ptr pixmapB;

	// Fetch input pixmaps
	pixmapA = imageCelA.cel()->getWorkingCopy(CEL_Representation::PIXMAP_AREA_RGBA16);
	pixmapB = imageCelB.cel()->getWorkingCopy(CEL_Representation::PIXMAP_AREA_RGBA16);

	// Composite pixmaps onto eachother
	try {
		if (_selection->value(context.frame()) == 1)
			CelAlgo::composite(pixmapB.get(), pixmapA.get(), CelAlgo::GC_OVERLAY_OVER, CelAlgo::GC_ADD);
		else
			CelAlgo::composite(pixmapA.get(), pixmapB.get(), CelAlgo::GC_OVERLAY_OVER, CelAlgo::GC_ADD);
	}
	catch (CelAlgo::AlgoException ex)
	{
		printf(ex.what().toLatin1().data());
		printf("\n");
	}

	// Move the result pixmap into the new cel
	if (_selection->value(context.frame()) == 1)
		result->setRepresentation(pixmapB);
	else
		result->setRepresentation(pixmapA);

	// If we have a valid cel then we can render it!
	if (result)
	{
		MO_SoftRenderServices::setCel(context, result, {MO_SoftRenderServices::DepthBufferStrategy::eCopyDepthBuffer, MO_SoftRenderServices::eCopyMessages} );
	}
}

REGISTER_CLASS(MO_ModuleFactorySingleton, FlipComp, FlipComp::Keyword())
