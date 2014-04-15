#ifndef IBONEMODIFIER_H
#define IBONEMODIFIER_H

class Matrix;
class BoneArray;

class IBoneModifier
{
public:
	virtual ~IBoneModifier() {}

	virtual void	Modify( const BoneArray* pBones, Matrix* pInOutBoneMatrices ) = 0;
};

#endif // IBONEMODIFIER_H