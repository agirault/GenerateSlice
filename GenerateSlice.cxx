#include <vtkImageData.h>
#include <vtkIntArray.h>
#include <vtkSmartPointer.h>
#include <vtkImageResize.h>
#include <vtkImageReslice.h>

vtkSmartPointer<vtkImageData> GenerateSlice(vtkImageData* inputData,
                                            int slice,
                                            int axis = 2,
                                            int sizeX = -1,
                                            int sizeY = -1)
{
  // Ensure axis is valid (I,J,K)
  if (axis < 0 || axis > 2)
  {
    return nullptr;
  }

  // Ensure slice number is within extent
  int extent[6];
  inputData->GetExtent(extent);
  if (slice < extent[axis*2] || slice > extent[axis*2+1])
  {
    return nullptr;
  }

  // Update output extent to select this slice
  extent[axis*2] = slice;
  extent[axis*2+1] = slice;

  // Reslice
  vtkSmartPointer<vtkImageReslice> reslice =
    vtkSmartPointer<vtkImageReslice>::New();
  reslice->SetOutputExtent(extent);
  reslice->SetInputData(inputData);
  reslice->Update();

  // Lambda to return a shallow copy of the output to free the filters
  auto getShallowCopyOfFilterOutput = [](vtkImageAlgorithm* filter)
  {
    vtkSmartPointer<vtkImageData> outputCopy =
      vtkSmartPointer<vtkImageData>::New();
    outputCopy->ShallowCopy(filter->GetOutput());
    return outputCopy;
  };

  // Return right away if no resizing necessary
  if (sizeX < 0 || sizeY < 0)
  {
    return getShallowCopyOfFilterOutput(reslice);
  }

  // Check sizes
  int iDims[3], oDims[3];
  oDims[axis] = 1;
  if (axis == 0)
  {
    oDims[1] = sizeX;
    oDims[2] = sizeY;
  }
  else if (axis == 1)
  {
    oDims[0] = sizeX;
    oDims[2] = sizeY;
  }
  else if (axis == 2)
  {
    oDims[0] = sizeX;
    oDims[1] = sizeY;
  }
  reslice->GetOutput()->GetDimensions(iDims);
  if (iDims[0] == oDims[0] &&
      iDims[1] == oDims[1] &&
      iDims[2] == oDims[2])
  {
    return getShallowCopyOfFilterOutput(reslice);
  }

  // Resize
  vtkSmartPointer<vtkImageResize> resize =
    vtkSmartPointer<vtkImageResize>::New();
  resize->SetOutputDimensions(oDims);
  resize->SetInputData(reslice->GetOutput());
  resize->Update();

  return getShallowCopyOfFilterOutput(resize);
}

int main(int, char *[])
{
  vtkSmartPointer<vtkImageData> image =
    vtkSmartPointer<vtkImageData>::New();
  image->SetExtent(0, 200, 0, 100, 0, 150);
  image->SetSpacing(0.1, 0.2, 0.15);
  image->AllocateScalars(VTK_INT,1);

  vtkSmartPointer<vtkImageData> output = GenerateSlice(image, 100, 1, 10, 10);
  assert(output != nullptr);
  output->Print(std::cout);

  return EXIT_SUCCESS;
}
