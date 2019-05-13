#include <vtkImageData.h>
#include <vtkIntArray.h>
#include <vtkSmartPointer.h>
#include <vtkImageReslice.h>

vtkSmartPointer<vtkImageData> GenerateSlice(vtkImageData* inputData, int axis, int slice)
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

  // Use ShallowCopy to free the reslice filter
  vtkSmartPointer<vtkImageData> output =
    vtkSmartPointer<vtkImageData>::New();
  output->ShallowCopy(reslice->GetOutput());

  return output;
}

int main(int, char *[])
{
  vtkSmartPointer<vtkImageData> image =
    vtkSmartPointer<vtkImageData>::New();
  image->SetExtent(0, 200, 0, 100, 0, 150);
  image->SetSpacing(0.1, 0.2, 0.15);
  image->AllocateScalars(VTK_INT,1);

  vtkSmartPointer<vtkImageData> output = GenerateSlice(image, 0, 100);
  assert(output != nullptr);
  output->Print(std::cout);

  return EXIT_SUCCESS;
}
