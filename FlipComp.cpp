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
: _selection(new AT_IntAttr(this, g_selection, true, 0, 0, 1))
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
		if (_selection->value(context.frame()) == 0)
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
	// Should we even be trying?
	if (!enabled())
	{
		setEmptyCel(context, 0);
	}

	// Output cel pointer
	CEL_Cel* result;

	// Cel Refs for A and B
	CA_CelRef imageCelA;
	CA_CelRef imageCelB;

	// These takes care of blank inputs, othewise this go VERY wrong
	if (MO_SoftRenderServices::handleInputPort(context, imageCelA,
		/*allowBlank*/ false,
		/*needMatPixmap*/ true,
		{ MO_SoftRenderServices::DepthBufferStrategy::eCopyDepthBuffer, MO_SoftRenderServices::eCopyMessages }))
	return;

	if (MO_SoftRenderServices::handleInputPort(context, imageCelB,
		/*allowBlank*/ false,
		/*needMatPixmap*/ true,
		{ MO_SoftRenderServices::DepthBufferStrategy::eCopyDepthBuffer, MO_SoftRenderServices::eCopyMessages }))
	return;

	// We NEED to have cels, so double check everything is valid
	if (!context.inputPortContext(0).cel() || !context.inputPortContext(1).cel())
	{
		// no input
		return;
	}

	// Grab cel refs from input ports
	imageCelA = context.inputPortContext(0).cel();
	imageCelB = context.inputPortContext(1).cel();

	// Make new output cel
	result = new CEL_Cel;

	// Pixmaps for A and B inputs
	CEL_PixmapAreaRGBA16::Ptr pixmapA;
	CEL_PixmapAreaRGBA16::Ptr pixmapB;

	// Fetch input pixmaps from the cels as 16 bit RGBA
	if (_selection->value(context.frame()) == 0)
	{
		// No flip
		pixmapA = imageCelA.cel()->getWorkingCopy(CEL_Representation::PIXMAP_AREA_RGBA16);
		pixmapB = imageCelB.cel()->getWorkingCopy(CEL_Representation::PIXMAP_AREA_RGBA16);
	} else {
		// FLIP!
		pixmapA = imageCelB.cel()->getWorkingCopy(CEL_Representation::PIXMAP_AREA_RGBA16);
		pixmapB = imageCelA.cel()->getWorkingCopy(CEL_Representation::PIXMAP_AREA_RGBA16);
	}

	// Composite pixmaps onto eachother
	try {
		// Blending modes are basic here, overlay over one and don't touch alpha, this composite is basically a no op
		CelAlgo::composite(pixmapB.get(), pixmapA.get(), CelAlgo::GC_OVERLAY_OVER, CelAlgo::GC_NOP);
	}
	catch (CelAlgo::AlgoException ex)
	{
		// Catch composite errors
		printf(ex.what().toLatin1().data());
		printf("\n");
	}

	// Move the result pixmap into the new cel
	result->setRepresentation(pixmapB);

	// If we have a valid cel then we can render it!
	if (result)
	{
		MO_SoftRenderServices::setCel(context, result, {MO_SoftRenderServices::DepthBufferStrategy::eCopyDepthBuffer, MO_SoftRenderServices::eCopyMessages} );
	}

	// Done!
}

REGISTER_CLASS(MO_ModuleFactorySingleton, FlipComp, FlipComp::Keyword())
