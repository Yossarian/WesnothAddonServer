from django.conf import settings
from django.contrib.auth.models import User, check_password

class PhpbbBackend:
	def authenticate(self, username=None, password=None):
		login_valid = (username == 'master')
		pwd_valid = (password == 'master')
		if login_valid and pwd_valid:
			try:
				user = User.objects.get(username=username)
				user.is_staff = True
			except User.DoesNotExist:
					user = User(username=username, password='whatever')
					user.save()
			return user
		return None

	def get_user(self, user_id):
		try:
			return User.objects.get(pk=user_id)
		except User.DoesNotExist:
			return None
