
from torch import nn

class ImageFilter(nn.Module):
	"""
	Filters (blurs) components of a tensor.
	
	Function to test io with TopLink tensors.
	"""
	def __init__(self, in_channels=4, out_channels=4, kernel_size=3, stride=1, groups=4):
		super().__init__()

		self.conv = nn.Conv2d(in_channels, out_channels, kernel_size, stride=stride, 
							  padding=kernel_size // 2, groups=groups, bias=False)

		nn.init.constant_(self.conv.weight, 1.0 / (kernel_size ** 2) )
		# nn.init.normal_(self.conv.weight, 0.0, 1)
		# nn.init.xavier_uniform_(self.conv.weight, gain=2.0)

	def forward(self, x):
		# Assuming x is of shape [batch_size, channels, height, width]
		return self.conv(x)

	def normalize(self, tensor):
		tensor_min = tensor.min()
		tensor_max = tensor.max()
		normalized_tensor = (tensor - tensor_min) / (tensor_max - tensor_min)
		return normalized_tensor
