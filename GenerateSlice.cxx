#include <vtkImageData.h>
#include <vtkImageResize.h>
#include <vtkImageReslice.h>
#include <vtkMatrix4x4.h>
#include <vtkPNGWriter.h>
#include <vtkSmartPointer.h>
#include <vtkXMLImageDataReader.h>

vtkSmartPointer<vtkImageData> GenerateSlice(vtkImageData* inputData,
                                            int slice,
                                            int axis = 2,
                                            int width = -1,
                                            int height = -1)
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

  // Set the slice orientation
  double origin[3];
  double spacing[3];
  inputData->GetOrigin(origin);
  inputData->GetSpacing(spacing);
  double sliceTranslation = origin[axis] + spacing[axis] * slice;
  auto axes = vtkSmartPointer<vtkMatrix4x4>::New();
  if (axis == 0)
  {
    const double sagittal[16] = {
      0.,  0., +1.,  sliceTranslation,
      -1.,  0.,  0., 0,
      0., -1.,  0.,  0.,
      0.,  0.,  0., +1.
    };
    axes->DeepCopy(sagittal);
  }
  else if (axis == 1)
  {
    const double coronal[16] = {
      +1.,  0.,  0.,  0.,
      0.,  0., +1.,  sliceTranslation,
      0., -1.,  0.,  0.,
      0.,  0.,  0., +1.
    };
    axes->DeepCopy(coronal);
  }
  if (axis == 2)
  {
    const double axial[16] = {
      +1.,  0.,  0.,  0.,
      0., +1.,  0.,  0.,
      0.,  0., +1.,  sliceTranslation,
      0.,  0.,  0., +1.
    };
    axes->DeepCopy(axial);
  }

  // Reslice
  vtkSmartPointer<vtkImageReslice> reslice =
    vtkSmartPointer<vtkImageReslice>::New();
  reslice->SetOutputDimensionality(2);
  reslice->SetResliceAxes(axes);
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
  if (width < 0 || height < 0)
  {
    return getShallowCopyOfFilterOutput(reslice);
  }

  // Check sizes
  int dims[3];
  reslice->GetOutput()->GetDimensions(dims);
  if (dims[0] == width && dims[1] == height)
  {
    return getShallowCopyOfFilterOutput(reslice);
  }

  // Resize
  dims[0] = width;
  dims[1] = height;
  vtkSmartPointer<vtkImageResize> resize =
    vtkSmartPointer<vtkImageResize>::New();
  resize->SetOutputDimensions(dims);
  resize->SetInputData(reslice->GetOutput());
  resize->Update();

  return getShallowCopyOfFilterOutput(resize);
}

int main(int, char *[])
{
  auto reader = vtkSmartPointer<vtkXMLImageDataReader>::New();
  reader->SetFileName("/path/to/file.vti");
  reader->Update();
  auto image = reader->GetOutput();
  image->Print(std::cout);

  auto output = GenerateSlice(image,
                              133, // slice number (range: extent along axis)
                              0,   // axis (I:0, J:1, K:2)
                              100, // 2d image width
                              100  // 2d image height
                              );
  assert(output != nullptr);
  output->Print(std::cout);

  auto pngWriter = vtkSmartPointer<vtkPNGWriter>::New();
  pngWriter->SetFileName("/path/to/outputImage.png");
  pngWriter->SetInputData(output);
  pngWriter->Write();

  return EXIT_SUCCESS;
}
